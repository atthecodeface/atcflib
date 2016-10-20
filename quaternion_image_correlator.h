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
 * @file          quaternion_image_correlator.h
 * @brief         Two-image correlator for rotation, scale (and shear later)
 *
 */

/*a Wrapper
 */
#ifdef __INC_QUATERNION_IMAGE_CORRELATOR
#else
#define __INC_QUATERNION_IMAGE_CORRELATOR

/*a Includes
 */
#include <vector>
#include <map>
#include "quaternion.h"
#include "filter.h"

/*a Defines
 */

/*a Types
 */
/*t t_qstm_count
 */
typedef struct
{
    int count;
    const class c_qi_src_tgt_match *qstm;
} t_qstm_count;

/*t t_qstm_score
 */
typedef struct
{
    double score;
    const class c_qi_src_tgt_match *qstm;
} t_qstm_score;

/*t t_quaternion_image_src_tgt_match_count_list
*/
typedef std::vector<t_qstm_count> t_quaternion_image_src_tgt_match_count_list;

/*t t_quaternion_image_src_tgt_match_list
*/
typedef std::vector<class c_qi_src_tgt_match *> t_quaternion_image_src_tgt_match_list;

/*t t_quaternion_image_src_tgt_match_score_list
*/
typedef std::vector<t_qstm_score> t_quaternion_image_src_tgt_match_score_list;

/*t t_quaternion_image_src_tgt_pair_mapping_list
 */
typedef std::vector<const class c_qi_src_tgt_pair_mapping *> t_quaternion_image_src_tgt_pair_mapping_list;

/*t t_quaternion_image_match_score
*/
typedef struct
{
    double score;
    t_quaternion_image_src_tgt_match_score_list match_list;
} t_quaternion_image_match_score;

/*c c_quaternion_image_correlator
*/
class c_quaternion_image_correlator
{
private:
    const c_quaternion *find_or_add_src_q(const c_quaternion *src_q);
    const c_quaternion *add_src_q(const c_quaternion *src_q);
    class c_qi_src_tgt_match *find_or_add_tgt_q_to_src_q(const c_quaternion *src_qx,
                                                         t_quaternion_image_src_tgt_match_list *src_tgt_match_list,
                                                         const c_quaternion *tgt_q);
    const c_quaternion *find_close_src_qx(const c_quaternion *src_q, double *cos_angle) const;
public:
    const c_quaternion *find_closest_src_qx(const c_quaternion *src_q, double *cos_angle) const;
    const class c_qi_src_tgt_match *find_closest_tgt_qx(const c_quaternion *src_qx, const c_quaternion *tgt_q, double *cos_angle) const;
    c_quaternion_image_correlator(void);
    int add_match(const c_quaternion *src_q,
                  const c_quaternion *tgt_q,
                  const t_point_value *pv);
    int create_mappings(void);
    double score_src_from_tgt(const c_quaternion *src_from_tgt_q);
    const c_quaternion *qstm_tgt_q(const class c_qi_src_tgt_match *qstm) const;
    const c_quaternion *qstm_src_q(const class c_qi_src_tgt_match *qstm) const;
    const c_quaternion *nth_src_tgt_q_mapping(const class c_qi_src_tgt_match *qstm,
                                              int n,
                                              const c_quaternion *src_tgt_qs[4]) const;
    const c_quaternion *qstpm_data(const class c_qi_src_tgt_pair_mapping *qstpm,
                                   const c_quaternion *src_tgt_qs[4]) const;
    const t_quaternion_image_src_tgt_match_count_list *best_matches_of_list(const t_quaternion_image_src_tgt_match_score_list *match_list,
                                                                            double min_score=0.0);
    const t_quaternion_image_src_tgt_pair_mapping_list *src_tgt_mappings_from_best_matches(const t_quaternion_image_src_tgt_match_count_list *best_matches,
                                                                                           int min_count=0);

    std::vector<const c_quaternion *> src_qs;
    std::map<const c_quaternion *, t_quaternion_image_src_tgt_match_list> matches_by_src_q;
    double min_cos_angle_src_q;
    double min_cos_angle_tgt_q;
    double min_cos_sep_score;
    double max_q_dist_score;
    double max_angle_diff_ratio;
    t_quaternion_image_match_score total_score;
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
