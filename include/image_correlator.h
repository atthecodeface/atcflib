/** Copyright (C) 2016,  Gavin J Stark.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file          image_correlator.h
 * @brief         Two-image correlator for rotation, scale (and shear later)
 *
 */

/*a Wrapper
 */
#ifdef __INC_IMAGE_CORRELATOR
#else
#define __INC_IMAGE_CORRELATOR

/*a Includes
 */
#include "filter.h"
#include <vector>
#include <string>

/*a Defines
 */
// a, b are -PI to PI.
// a-b is -2*PI, -PI, 0, PI, to 2*PI
// Want       1,   0, 1,  0,   1
// cos(a-b) is 1, -1, 1,  -1,  1
// 1+cos(a-b) is 2, 0, 2,  0,  2
#define ROTATION_DIFF_STRENGTH(a,b) ( (1+cos((a)-(b))) / 2 )
//#define FFT_ROTATION(dy,dx) (atan2(-(dy),(dx)))
//#define FFT_ROTATION(dy,dx) (atan2(-(dy),(dx)))
//#define FFT_ROTATION(dy,dx) (atan2(dy,-dx))
// after fixing base functions, try:
#define FFT_ROTATION(dy,dx) (-atan2(dy,dx))

// DIST_FACTOR used to be 2 or 4
//#define DIST_FACTOR (10.0)
#define DIST_FACTOR (2.0)

#define PI (M_PI)
#define DEG(a) (180/PI*(a))
#define RAD(a) ((a)*PI/180)

/*a Types
 */
/*t t_image_correlation_proposition
 */
typedef struct
{
    double translation[2];
    double rotation;
    double scale;
} t_image_correlation_proposition;

/*t c_image_correlator
 */
class c_image_correlator
{
private:
    double try_delta_proposition(double current_strength, t_image_correlation_proposition *proposition, t_image_correlation_proposition *delta, double scale, FILE *verbose);
    double tweak_proposition(t_image_correlation_proposition *proposition, double scale, FILE *verbose);
protected:    

public:
    c_image_correlator(void);
    ~c_image_correlator();

    int add_mapping_point(const char *name, double x, double y);
    class c_mapping_point *find_mapping_point(std::string name) const;
    int add_mapping_point_pv(const char *name, const char *pv_name, t_point_value *pv);
    const char *get_mapping_point_pv_name(const char *name, int n) const;
    t_point_value *get_mapping_point_pv(const char *name, const char *pv_name) const;
    int add_proposition(class c_mapping_point *mp0, class c_mapping_point *mp1, struct t_pv_match *pv0, struct t_pv_match *pv1, t_image_correlation_proposition *proposition, double strength);
    int add_proposition(const char *names[2], const char *pv_names[2], t_image_correlation_proposition *proposition, double strength);
    void show_strength_in_mapping(t_image_correlation_proposition *proposition, FILE *verbose);
    void diminish_mappings_by_proposition(t_image_correlation_proposition *proposition);
    double strength_of_proposition(t_image_correlation_proposition *proposition);
    double find_best_mapping(t_image_correlation_proposition *best_image_correlation_proposition, FILE *verbose);
    void rate_proposition(t_image_correlation_proposition *proposition);
    double create_proposition(c_mapping_point *mp0, c_mapping_point *mp1, struct t_pv_match *pv0, struct t_pv_match *pv1, t_image_correlation_proposition *proposition);
    void create_propositions(double min_strength, FILE *verbose);
    void show_mappings(void);
    void reset_diminishments(void);
    int number_of_propositions(const char *name);
    t_image_correlation_proposition *get_proposition(const char *name, int n);

    std::map<std::string, class c_mapping_point *> mapping_points;
};

/*a External functions
 */
/*a Wrapper
 */
#endif

/*a Editor preferences and notes
mode: c ***
c-basic-offset: 4 ***
c-default-style: (quote ((c-mode . "k&r") (c++-mode . "k&r"))) ***
outline-regexp: "/\\\*a\\\|[\t ]*\/\\\*[b-z][\t ]" ***
*/
