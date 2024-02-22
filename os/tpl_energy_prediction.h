/**
 * @file tpl_energy_prediction.h
 *
 * @section descr File description
 *
 * Trampoline sequence os services implementation.
 *
 * @section copyright Copyright
 *
 * Trampoline RTOS
 *
 * Trampoline is copyright (c) CNRS, University of Nantes, Ecole Centrale de
 * Nantes Trampoline is protected by the French intellectual property law.
 *
 * This software is distributed under the GNU Public Licence V2.
 * Check the LICENSE file in the root directory of Trampoline
 *
 * @section infos File informations
 *
 * $Date$
 * $Rev$
 * $Author$
 * $URL$
 *
 */

struct TPL_ENERGY
{
    VAR(uint16_t, TYPEDEF, OS_VAR) prediction;
    P2VAR(uint16_t, TYPEDEF, OS_VAR) previous_harvesting;
};

typedef struct TPL_ENERGY tpl_energy;

