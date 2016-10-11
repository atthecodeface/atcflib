/*a Documentation
 */
/*a Includes
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <map>
#include "image_correlator.h"

/*a Base types
 */
/*t t_pv_match
*/
typedef struct t_pv_match
{
    t_point_value pv;
    double rotation;
} t_pv_match;

/*c c_mapping
  Mappings are a pair of (source point -> target point) correlations
  A mapping may be believed strongly or not - they are not necessarily valid
  As such they have a proposition - a proposed rotation, translation etc and strength
  based on their input data.
  This proposition should be entirely derived from the data within the mapping
  i.e. it should be recreatable from the mapping provided

  Methods are needed to see how well this mapping matches other propositions; such
  other propositions are derived from one or more mappings.

  map_strength relates to how well does a proposition match this
  mapping (and therefore this mapping's proposition)

  position_map_strength relates to how well does the position of the
  points match a proposition.

  The difference is that the former compares the propositions directly.
  The latter maps source points to the target through the proposition, and compares
  the results (particularly of the first point in the mapping).

  Hence the former can be used to see which propositions inherently
  align, and the latter can be used to generate a degree of misfit for
  the target - since many different propositions MAY map a source point to a
  target point in the same way.

  So the latter allows for disagreeing with a proposition, the former
  for generating a consensus of a proposition to start with.

 */
class c_mapping
{
public:
    c_mapping(class c_mapping_point *src_pt,
              class c_mapping_point *src_other,
              t_pv_match *tgt_pv,
              t_pv_match *tgt_other_pv,
              t_image_correlation_proposition *proposition,
              double strength);
    double map_strength(t_image_correlation_proposition *proposition);
    double position_map_strength(t_image_correlation_proposition *proposition);
    void repr(char *buffer, int buf_size);

    class c_mapping_point *src_pts[2];
    t_pv_match *pvs[2];
    t_image_correlation_proposition proposition;
    double strength;
    double initial_strength;
};

/*c c_mapping_point
    A point on the source image has a set of mapping beliefs, each with a concept of 'strength'
    These can be its ideas that it maps to particular points on the target with an image rotation,
    or it can be its ideas that the mapping for an image is a rotation and scale followed by a translation (tx, ty)
    This latter is built up from pairs of points, and is purer in our application as the rotation is derived
    from the pair of points rather than the phase of the FFT (which is more of a clue than a calculation)

    The point can be asked for its most-trusted proposition that it has not been told to ignore. It may have none.
    The main algorithm can combine these propositions from all source points to come up with a 'consensus proposition'
    Each point is then asked to use the consensus proposition to produce a new most-trusted proposition (even using data it has been told to ignore I think)
    The points may have none.
    A consensus propositino is again reached - and we repeat to get a most-agreed consensus.
    At this point the main algorithm outputs the most-agreed consensus, and informs each
    point to ignore propositions that support this consensus
    Repeat from the beginning until we have enough consensus propositions or there are no propositions

*/
class c_mapping_point
{
public:
    c_mapping_point(double x, double y);
    ~c_mapping_point();

    void add_match(const char *match_name, t_point_value *pv);
    void add_mapping(c_mapping *mapping);
    c_mapping *find_strongest_belief(t_image_correlation_proposition *proposition, double *best_strength);
    double strength_in_belief(t_image_correlation_proposition *proposition);
    void diminish_belief_in_proposition(t_image_correlation_proposition *proposition);

    int number_of_propositions(void);
    t_image_correlation_proposition *get_proposition(int n);

    void __str__(char *buffer, int buf_size);

    std::map<std::string, t_pv_match> matches;
    std::vector<c_mapping *> mappings;
    double coords[2];
};

/*a c_mapping methods
 */
/*f c_mapping::c_mapping
 */
c_mapping::c_mapping(class c_mapping_point *src_pt, class c_mapping_point *src_other_pt,
                     t_pv_match *tgt_pv, t_pv_match *tgt_other_pv,
                     t_image_correlation_proposition *proposition, double strength)
{
    this->src_pts[0] = src_pt;
    this->src_pts[1] = src_other_pt;
    this->pvs[0] = tgt_pv;
    this->pvs[1] = tgt_other_pv;
    this->proposition = *proposition;
    this->initial_strength = strength;
    this->strength = strength;
}

/*f c_mapping::map_strength
  how well does a proposition match this mapping's proposition?
  Return 0 for poor, 1 for good, sliding scale
 */
double c_mapping::map_strength(t_image_correlation_proposition *proposition)
{
    double strength;
    double dx, dy, translation_dist;
    t_image_correlation_proposition *tp;
    tp = &(this->proposition);
    strength = this->strength;
    if (proposition->scale < tp->scale) {
        strength *= proposition->scale / tp->scale;
    } else {
        strength *= tp->scale / proposition->scale;
    }
    dx = tp->translation[0] - proposition->translation[0];
    dy = tp->translation[1] - proposition->translation[1];
    translation_dist = sqrt(dx*dx + dy*dy);
    strength *= DIST_FACTOR / (DIST_FACTOR + translation_dist);
    strength *= ROTATION_DIFF_STRENGTH(tp->rotation,proposition->rotation);
    return strength;
}

/*f c_mapping::position_map_strength
  how well does a proposition match the points being mapped?
  Return 0 for poor, 1 for good, sliding scale
 */
double c_mapping::position_map_strength(t_image_correlation_proposition *proposition)
{
    double proposition_strength;
    double src_x, src_y;
    double cosang, sinang;
    double tgt_x, tgt_y;
    double dx, dy, dist;

    cosang = cos(proposition->rotation);
    sinang = sin(proposition->rotation);

    proposition_strength = strength;
    for (int i=0; i<1; i++) { // Ignore other point in case it is mapped more than once - it should be caught anyhow?
        src_x = src_pts[0]->coords[0];
        src_y = src_pts[0]->coords[1];
        tgt_x = proposition->translation[0] + proposition->scale*(cosang*src_x - sinang*src_y);
        tgt_y = proposition->translation[1] + proposition->scale*(cosang*src_y + sinang*src_x);
        dx = tgt_x - pvs[0]->pv.x;
        dy = tgt_y - pvs[0]->pv.y;
        dist = sqrt(dx*dx+dy*dy);
        proposition_strength *= (DIST_FACTOR/(DIST_FACTOR+dist));
    }

    proposition_strength *= ROTATION_DIFF_STRENGTH(this->proposition.rotation, proposition->rotation);
    return proposition_strength;
}

/*f c_mapping::repr
 */
void c_mapping::repr(char *buffer, int buf_size)
{
    snprintf(buffer, buf_size, "(%4d,%4d) -> (%4d,%4d) : (%4d,%4d) -> (%4d,%4d) : (%8.2f,%8.2f) %6.2f %5.3f    %8.5f",
             (int) this->src_pts[0]->coords[0],
             (int) this->src_pts[0]->coords[1],
             (int) this->pvs[0]->pv.x,
             (int) this->pvs[0]->pv.y,
             (int) this->src_pts[1]->coords[0],
             (int) this->src_pts[1]->coords[1],
             (int) this->pvs[1]->pv.x,
             (int) this->pvs[1]->pv.y,
             this->proposition.translation[0],
             this->proposition.translation[1],
             DEG(this->proposition.rotation),
             this->proposition.scale,
             this->strength);
    buffer[buf_size-1] = 0;
}

/*a c_mapping_point methods
 */
/*f c_mapping_point::c_mapping_point
 */
c_mapping_point::c_mapping_point(double x, double y)
{
    coords[0] = x;
    coords[1] = y;
}

/*f c_mapping_point::~c_mapping_point
 */
c_mapping_point::~c_mapping_point()
{
    //delete matches;
    //delete mappings;
}

/*f c_mapping_point::add_match
 */
void c_mapping_point::add_match(const char *match_name, t_point_value *pv)
{
    t_pv_match pv_match;
    pv_match.pv = *pv;
    pv_match.rotation = FFT_ROTATION(pv->vec_y, pv->vec_x);
    matches[std::string(match_name)] = pv_match;
}

/*f c_mapping_point::add_mapping
 */
void c_mapping_point::add_mapping(c_mapping *mapping)
{
    mappings.push_back(mapping);
}

/*f c_mapping_point::find_strongest_belief
 */
c_mapping *c_mapping_point::find_strongest_belief(t_image_correlation_proposition *proposition, double *best_strength)
{
    c_mapping *best_mapping;
    double max_strength;

    max_strength = 0;
    best_mapping = NULL;
    for (auto m : mappings) {
        double s;
        if (proposition) {
            s = m->position_map_strength(proposition);
        } else {
            s = m->strength;
        }
        if (s>max_strength) {
            max_strength = s;
            best_mapping = m;
        }
    }
    *best_strength = max_strength;
    return best_mapping;
}

/*f c_mapping_point::strength_in_belief
 */
double c_mapping_point::strength_in_belief(t_image_correlation_proposition *proposition)
{
    double strength = 0;
    for (auto m : mappings) {
        strength += m->position_map_strength(proposition);
    }
    return strength;
}
    
/*f c_mapping_point::diminish_belief_in_proposition
 */
void c_mapping_point::diminish_belief_in_proposition(t_image_correlation_proposition *proposition)
{
    for (auto m : mappings) {
        double s;
        s = m->position_map_strength(proposition);
        //fprintf(stderr,"Diminish %p %f %f\n", m, s, m->strength);
        if (s>0) {
            s /= m->strength;
            m->strength *= (1-s)*(1-s);
        }
    }
}

/*f c_mapping_point::number_of_propositions
*/
int
c_mapping_point::number_of_propositions(void)
{
    int n=0;
    for (auto m : mappings) {
        n++;
        (void) m;
    }
    return n;
}

/*f c_mapping_point::get_proposition
*/
t_image_correlation_proposition *c_mapping_point::get_proposition(int n)
{
    for (auto m : mappings) {
        if (n==0) return &(m->proposition);
        n--;
    }
    return NULL;
}

/*a Image correlator
*/
/*f c_image_correlator constructor
*/
c_image_correlator::c_image_correlator(void)
{
}

c_image_correlator::~c_image_correlator()
{
}

/*f c_image_correlator::diminish_mappings_by_proposition
 */
void 
c_image_correlator::diminish_mappings_by_proposition(t_image_correlation_proposition *proposition)
{
    for (auto mp : mapping_points) {
        mp.second->diminish_belief_in_proposition(proposition);
    }
}

/*f c_image_correlator::strength_of_proposition
 */
double
c_image_correlator::strength_of_proposition(t_image_correlation_proposition *proposition)
{
    double total_strength;
    total_strength = 0;
    for (auto mp : mapping_points) {
        double s_in_b;
        double s_in_m;
        s_in_b = mp.second->strength_in_belief(proposition);
        c_mapping *m = mp.second->find_strongest_belief(proposition, &s_in_m);
        if (m) { total_strength += s_in_m; }
        //total_strength += s_in_b;
    }
    return total_strength;
}

/*f c_image_correlator::try_delta_proposition
 */
double
c_image_correlator::try_delta_proposition(double current_strength, t_image_correlation_proposition *proposition, t_image_correlation_proposition *delta, double scale, FILE *verbose)
{
    double delta_strength;
    t_image_correlation_proposition keep;
    keep = *proposition;
    proposition->translation[0] += scale*delta->translation[0];
    proposition->translation[1] += scale*delta->translation[1];
    proposition->rotation       += scale*delta->rotation;
    proposition->scale          += scale*delta->scale;
    delta_strength = strength_of_proposition(proposition);
    if (delta_strength>current_strength)
    {
        if (verbose) {
            fprintf(verbose,"Improved strength %8.4f from %8.4f: Translation (%8.2f,%8.2f) rotation %6.2f scale %6.4f\n",
                    delta_strength, current_strength, proposition->translation[0], proposition->translation[1], DEG(proposition->rotation), proposition->scale);
        }
        current_strength = delta_strength;
    } else {
        *proposition = keep;
    }
    return current_strength;
}

/*f c_image_correlator::tweak_proposition
 */
double
c_image_correlator::tweak_proposition(t_image_correlation_proposition *proposition, double scale, FILE *verbose)
{
    double total_strength;
    t_image_correlation_proposition delta_cp;

    total_strength = strength_of_proposition(proposition);

    delta_cp.translation[0] = 0;
    delta_cp.translation[1] = 0;
    delta_cp.scale = 0;
    delta_cp.rotation = 0;

    delta_cp.scale = 0.0;
    delta_cp.rotation = RAD(0.3);

    total_strength = try_delta_proposition(total_strength, proposition, &delta_cp,  1.0*scale, verbose );
    total_strength = try_delta_proposition(total_strength, proposition, &delta_cp, -1.0*scale, verbose );

    delta_cp.scale = 0.0;
    delta_cp.rotation = RAD(0.1);

    total_strength = try_delta_proposition(total_strength, proposition, &delta_cp,  1.0*scale, verbose );
    total_strength = try_delta_proposition(total_strength, proposition, &delta_cp, -1.0*scale, verbose );

    delta_cp.translation[0] = 0.1;
    delta_cp.translation[1] = 0;

    total_strength = try_delta_proposition(total_strength, proposition, &delta_cp,  1.0*scale, verbose );
    total_strength = try_delta_proposition(total_strength, proposition, &delta_cp, -1.0*scale, verbose );

    delta_cp.translation[0] = 0;
    delta_cp.translation[1] = 0.1;

    total_strength = try_delta_proposition(total_strength, proposition, &delta_cp,  1.0*scale, verbose );
    total_strength = try_delta_proposition(total_strength, proposition, &delta_cp, -1.0*scale, verbose );

    return total_strength;
}

/*f c_image_correlator::show_strength_in_mapping
 */
void
c_image_correlator::show_strength_in_mapping(t_image_correlation_proposition *proposition, FILE *verbose)
{
    double total_strength_in_belief, total_strength_in_max;

    total_strength_in_belief = 0;
    total_strength_in_max = 0;
    for (auto mp : mapping_points) {
        double s_in_b;
        double s_in_m;
        c_mapping *m;
        s_in_b = mp.second->strength_in_belief(proposition);
        total_strength_in_belief += s_in_b;
        m = mp.second->find_strongest_belief(proposition, &s_in_m);
        if (m) {
            if (verbose) {
                char buf[256];
                m->repr(buf, sizeof(buf));
                fprintf(verbose, "%s %f %f %s\n", mp.first.c_str(), s_in_b, s_in_m, buf);
            }
            total_strength_in_max += s_in_m;
        }
    }
    fprintf(stderr,"Strength %8.4f / %8.4f: Translation (%8.2f,%8.2f) rotation %6.2f scale %6.4f\n\n",
            total_strength_in_belief, total_strength_in_max,
            proposition->translation[0], proposition->translation[1], DEG(proposition->rotation), proposition->scale);
}

/*f c_image_correlator::find_best_mapping
 */
double
c_image_correlator::find_best_mapping(t_image_correlation_proposition *best_image_correlation_proposition, FILE *verbose)
{
    double best_mapping_strength;
    c_mapping *best_mapping;
    t_image_correlation_proposition cp;
    double total_strength;

    /*b Find best mapping to start with
     */
    best_mapping_strength = 0;
    best_mapping = NULL;
    for (auto mp : mapping_points) {
        double strength;
        c_mapping *m = mp.second->find_strongest_belief(NULL, &strength);
        if (m && (strength>best_mapping_strength)) {
            best_mapping_strength = strength;
            best_mapping = m;
        }
    }
    if (!best_mapping) return 0;

    /*b Starting with best mapping, iterate to improve - at least perturb from the max of a perfect match
     */
    cp = best_mapping->proposition;
    total_strength = 0;
    for (int i=0; i<1; i++) {
        t_image_correlation_proposition np;
        double np_dx, np_dy;
        np.translation[0] = 0;
        np.translation[1] = 0;
        np.rotation = 0;
        np_dy = 0;
        np_dx = 0;
        np.scale = 0;
        total_strength = 0;
        for (auto mp : mapping_points) {
            double s_in_b;
            double s_in_m;
            c_mapping *m;
            s_in_b = mp.second->strength_in_belief(&cp);
            m = mp.second->find_strongest_belief(&cp, &s_in_m);
            if (m) {
                double s;
                s = s_in_b;
                np.translation[0] += s*m->proposition.translation[0];
                np.translation[1] += s*m->proposition.translation[1];
                np.scale     += s*m->proposition.scale;
                np_dx        += s*cos(m->proposition.rotation);
                np_dy        += s*sin(m->proposition.rotation);
                total_strength += s;
                if (0) {
                    char buf[256];
                    m->repr(buf, sizeof(buf));
                    fprintf(stderr, "%s %f %f %s\n", mp.first.c_str(), s_in_b, s_in_m, buf);
                }
            }
        }
        if (total_strength==0) return 0;
        cp.translation[0] = np.translation[0]/total_strength;
        cp.translation[1] = np.translation[1]/total_strength;
        cp.scale     = np.scale/total_strength;
        cp.rotation  = atan2(np_dy, np_dx);
    }

    if (0) {
        fprintf(stderr,"Pre-tweak strength %8.4f: Translation (%8.2f,%8.2f) rotation %6.2f scale %6.4f\n\n",
                total_strength, cp.translation[0], cp.translation[1], DEG(cp.rotation), cp.scale);
    }
    for (int i=0; i<30; i++) {
        double last_strength = total_strength;
        total_strength = tweak_proposition(&cp, 10.0, verbose);
        if (total_strength<=last_strength) break;
    }
    for (int i=0; i<30; i++) {
        double last_strength = total_strength;
        total_strength = tweak_proposition(&cp, 1.0, verbose);
        if (total_strength<=last_strength) break;
    }
    for (int i=0; i<30; i++) {
        double last_strength = total_strength;
        total_strength = tweak_proposition(&cp, 0.1, verbose);
        if (total_strength<=last_strength) break;
    }
    if (0) {
        show_strength_in_mapping(&cp, verbose);
    }

    *best_image_correlation_proposition = cp;

    fprintf(stderr,"Strength %8.4f: Translation (%8.2f,%8.2f) rotation %6.2f scale %6.4f\n",
            total_strength,
            best_image_correlation_proposition->translation[0], best_image_correlation_proposition->translation[1],
            DEG(best_image_correlation_proposition->rotation), best_image_correlation_proposition->scale );
    fprintf(stderr,"Gimp: scale %8.4f; rotate about center %8.4f; offset (%8.2f,%8.2f)\n\n",
            best_image_correlation_proposition->scale, DEG(best_image_correlation_proposition->rotation),
            best_image_correlation_proposition->translation[0]-512 + 512*best_image_correlation_proposition->scale*(cos(best_image_correlation_proposition->rotation)-sin(best_image_correlation_proposition->rotation)),
            best_image_correlation_proposition->translation[1]-512 + 512*best_image_correlation_proposition->scale*(cos(best_image_correlation_proposition->rotation)+sin(best_image_correlation_proposition->rotation)) );

    return total_strength;

}

/*f c_image_correlator::rate_proposition
 */
void
c_image_correlator::rate_proposition(t_image_correlation_proposition *proposition)
{
    for (auto mp : mapping_points) {
        char buf[256];
        double s;
        fprintf(stderr,"Mapping point %s\n",mp.first.c_str());
        for (auto m : mp.second->mappings) {
            m->repr(buf,sizeof(buf));
            s = m->position_map_strength(proposition);
            fprintf(stderr, "%f: %s\n", s, buf);
        }
    }
}

/*f c_image_correlator::show_mappings
 */
void
c_image_correlator::show_mappings(void)
{
    for (auto mp : mapping_points) {
        fprintf(stderr,"Mapping point %s\n",mp.first.c_str());
        char buf[256];
        double strength;
        c_mapping *m = mp.second->find_strongest_belief(NULL, &strength);
        fprintf(stderr,"  Mapping %p %f\n",m,strength);
        if (m) {
            m->repr(buf,sizeof(buf));
            fprintf(stderr, "%f, %s\n", strength, buf);
        }
    }
}

/*f c_image_correlator::reset_diminishments
 */
void
c_image_correlator::reset_diminishments(void)
{
    for (auto mp : mapping_points) {
        for (auto m : mp.second->mappings) {
            m->strength = m->initial_strength;
        }
    }
}

/*f c_image_correlator::add_mapping_point
 */
int
c_image_correlator::add_mapping_point(const char *name, double x, double y)
{
    c_mapping_point *mp;
    mp = new c_mapping_point(x, y);
    if (!mp) return -1;
    mapping_points[name] = mp;
    return 0;
}

/*f c_image_correlator::find_mapping_point
 */
c_mapping_point *
c_image_correlator::find_mapping_point(std::string name) const
{
    if (mapping_points.count(name)==0)
        return NULL;
    return mapping_points.find(name)->second;
}

/*f c_image_correlator::add_mapping_point_pv
 */
int
c_image_correlator::add_mapping_point_pv(const char *name, const char *match_name, t_point_value *pv)
{
    c_mapping_point *mp;
    mp = find_mapping_point(name);
    if (!mp)
        return -1;
    mp->add_match(match_name, pv);
    return 0;
}

/*f c_image_correlator::get_mapping_point_pv_name
*/
const char *c_image_correlator::get_mapping_point_pv_name(const char *name, int n) const
{
    c_mapping_point *m;
    m = find_mapping_point(name);
    for (auto pv : m->matches) {
        if (n==0) return pv.first.c_str();
        n--;
    }
    return NULL;
}

/*f c_image_correlator::get_mapping_point_pv
*/
t_point_value *
c_image_correlator::get_mapping_point_pv(const char *name, const char *pv_name) const
{
    c_mapping_point *m;
    m = find_mapping_point(name);
    if (m->matches.count(pv_name)==0)
        return NULL;
    return &(m->matches[pv_name].pv);
}

/*f c_image_correlator::add_proposition(names...)
 */
int
c_image_correlator::add_proposition(const char *names[2], const char *pv_names[2], t_image_correlation_proposition *proposition, double strength)
{
    c_mapping_point *mps[2];
    t_pv_match *pvs[2];
    for (int i=0; i<2; i++) {
        mps[i] = find_mapping_point(names[i]);
        if (!mps[i]) return -1;
        pvs[i] = &(mps[i]->matches[pv_names[i]]);
    }

    return add_proposition(mps[0], mps[1], pvs[0], pvs[1], proposition, strength);
}

/*f c_image_correlator::add_proposition
 */
int
c_image_correlator::add_proposition(c_mapping_point *mp0, c_mapping_point *mp1, t_pv_match *pv0, t_pv_match *pv1, t_image_correlation_proposition *proposition, double strength)
{
    c_mapping *m = new c_mapping(mp0, mp1, pv0, pv1, proposition, strength);
    if (!m) return -1;
    mp0->add_mapping(m);
    return 0;
}

/*f c_image_correlator::create_proposition
*/
double
c_image_correlator::create_proposition(c_mapping_point *mp0, c_mapping_point *mp1, t_pv_match *pv0, t_pv_match *pv1, t_image_correlation_proposition *proposition)
{
    double tgt_dx, tgt_dy, tgt_l;
    double src_dx, src_dy, src_l;
    double scale, rotation;
    double cos_rot_diff;

    /*b Check PV rotations are approximately inline
     */
    cos_rot_diff = cos(pv0->rotation - pv1->rotation);
    if (cos_rot_diff<0.90)
        return -1;

    /*b Determine scale and rotation given both mappings
     */
    tgt_dx = pv0->pv.x - pv1->pv.x;
    tgt_dy = pv0->pv.y - pv1->pv.y;
    src_dx = mp0->coords[0] - mp1->coords[0];
    src_dy = mp0->coords[1] - mp1->coords[1];

    tgt_l = sqrt(tgt_dx*tgt_dx + tgt_dy*tgt_dy);
    src_l = sqrt(src_dx*src_dx + src_dy*src_dy);

    scale = tgt_l / src_l;

    rotation = atan2(tgt_dy, tgt_dx) - atan2(src_dy, src_dx);
    rotation = (rotation<-PI) ? (rotation+2*PI) : rotation;
    rotation = (rotation>PI) ? (rotation-2*PI) : rotation;

    /*b If scale is out of bounds, then drop
     */
    if ((scale<0.95) || (scale>1.05))
        return -1;

    /*b Check FFT rotation matches actual rotation, give or take
      There is a mapping src -> tgt that is tgt = scale*rotation*src + translation
      Hence (m.tgt - pm.tgt) = scale * rotation * (m.src-pm.src)
      or tgt_dxy = scale*rotation*(src_dxy)
      Hence (tgt.dxy).(src.dxy) = (scale*rotation*(src.dxy)) . (src.dxy)
      Hence (tgt.dxy).(src.dxy) = scale*cos(rotation)*((src.dxy).(src.dxy))
      Hence scale*cos(rotation) = (tgt.dxy).(src.dxy) / (src_l^2)
      Hence scale*cos(rotation) = (tgt_l * src_l * cosang) / (src_l^2)
      Hence scale*cos(rotation) = tgt_l/src_l * cosang
      Hence cos(rotation) should match cosang, and scale is tgt_l/src_l
      Note that if two angles are approximately equal, then cos(diff) is near 1.0
      angdiff should be about rotation
    */
    cos_rot_diff = cos(pv0->rotation - rotation);
    if (cos_rot_diff<0.90)
        return -1;

    /*b tgt_pt = scale*rotation*src_pt + translation
      Hence translation = tgt_pt - scale*rotation*src_pt
    */
    double cos_rot, sin_rot;
    double translation_x, translation_y;

    cos_rot = cos(rotation);
    sin_rot = sin(rotation);
    translation_x = pv0->pv.x - scale*(cos_rot*mp0->coords[0] - sin_rot*mp0->coords[1]);
    translation_y = pv0->pv.y - scale*(cos_rot*mp0->coords[1] + sin_rot*mp0->coords[0]);

    proposition->translation[0] = translation_x;
    proposition->translation[1] = translation_y;
    proposition->rotation = rotation;
    proposition->scale = scale;

    return 100*pv0->pv.value*pv1->pv.value;
}

/*f c_image_correlator::create_propositions
 */
void
c_image_correlator::create_propositions(double min_strength, FILE *verbose)
{
    for (auto mp0 : mapping_points) {
        for (auto mp1 : mapping_points) {
            for (auto pv0 : mp0.second->matches) {
                for (auto pv1 : mp1.second->matches) {
                        t_image_correlation_proposition proposition;
                        double strength = create_proposition(mp0.second, mp1.second, &pv0.second, &pv1.second, &proposition);
                        if (strength>=min_strength) {
                            add_proposition(mp0.second, mp1.second, &pv0.second, &pv1.second, &proposition, strength);                            
                        }
                        /*
                            fprintf(verbose, "Dropping (%d,%d)->(%d,%d) and (%d,%d)->(%d,%d) as rotations mismatch (%f, %f, %f)\n",
                                    (int)mp0->coords[0], (int)mp0->coords[1], pv0->pv.x, pv0->pv.y,
                                    (int)mp1->coords[0], (int)mp1->coords[1], pv1->pv.x, pv1->pv.y,
                                    DEG(pv0->rotation), DEG(pv1->rotation), cos_rot_diff );

                        if (0) {
                            fprintf(stderr,"Adding\n");
                            fprintf(stderr," (%d,%d) -> (%d,%d)  and   (%d,%d) -> (%d,%d)\n",
                                    (int)mp0->coords[0], (int)mp0->coords[1], pv->x, pv->y,
                                    (int)pm->coords[0], (int)pm->coords[1], pmpv->pv.x, pmpv->pv.y);
                            fprintf(stderr,"   src_dxy (%f,%f) tgt_dxy (%f,%f) src_l %f tgt_l %f\n",
                                    src_dx, src_dy, tgt_dx, tgt_dy, src_l, tgt_l );
                            fprintf(stderr,"   scale %f rotation %f fft_rotation %f pm_fft_rotation %f\n",
                                    scale, DEG(rotation), DEG(fft_rotation), DEG(pmpv->rotation) );
                            fprintf(stderr,"   cos_rot_diff %f\n",
                                    cos_rot_diff );
                        */
                }
            }
        }
    }
}

/*f c_image_correlator::number_of_propositions
*/
int
c_image_correlator::number_of_propositions(const char *name)
{
    c_mapping_point *mp;
    mp = find_mapping_point(name);
    if (!mp) return 0;
    return mp->number_of_propositions();
}

/*f c_image_correlator::get_proposition
*/
t_image_correlation_proposition
*c_image_correlator::get_proposition(const char *name, int n)
{
    c_mapping_point *mp;
    mp = find_mapping_point(name);
    if (!mp) return NULL;
    return mp->get_proposition(n);
}

