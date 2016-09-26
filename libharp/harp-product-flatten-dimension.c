/*
 * Copyright (C) 2015-2016 S[&]T, The Netherlands.
 *
 * This file is part of HARP.
 *
 * HARP is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * HARP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HARP; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "harp-internal.h"
#include "harp-constants.h"
#include "harp-csv.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/** Collapse a given dimension into the time dimension
 * 
 * Flattening a product for a certain dimension collapses the dimension into the time dimension (i.e. the time
 * dimension and the provided dimension are flattened together).
 * For instance, if a product contains a variable with [num_time,num_longitude,num_latitudes,num_vertical] as
 * dimensions, then flattening for the vertical dimension will result in a variable with
 * [num_time*num_vertical,num_longitudes,num_latitudes] as dimensions.
 *
 * The end result of this function is the time dimension will have grown by a factor equal to the length of the given
 * dimension type and that none of the variables in the product will depend on the given dimension type anymore.
 *
 * Any variables that depend more than once on the given dimension type will be removed from the product.
 * The index and collocation_index variables will be removed if present.
 * Variables that had the given dimension type but were time independent are first made time dependent before
 * flattening the dimension.
 *
 * Independent dimensions cannot be flattened.
 * \param product HARP product.
 * \param dimension_type Dimension to use for the flattening.
 * \return
 *   \arg \c 0, Success.
 *   \arg \c -1, Error occurred (check #harp_errno).
 */
LIBHARP_API int harp_product_flatten_dimension(harp_product *product, harp_dimension_type dimension_type)
{
    harp_variable *var;
    long dim_length = product->dimension[dimension_type];
    int i;

    if (dimension_type == harp_dimension_independent)
    {
        harp_set_error(HARP_ERROR_INVALID_ARGUMENT, "cannot flatten independent dimension");
        return -1;
    }
    if (dim_length == 0 || dimension_type == harp_dimension_time)
    {
        return 0;
    }

    /* remove index and collocation_index variables if they exist */
    if (harp_product_get_variable_by_name(product, "index", &var) != -1)
    {
        if (harp_product_remove_variable(product, var) != 0)
        {
            return -1;
        }
    }
    if (harp_product_get_variable_by_name(product, "collocation_index", &var) != -1)
    {
        if (harp_product_remove_variable(product, var) != 0)
        {
            return -1;
        }
    }

    for (i = product->num_variables - 1; i >= 0; i--)
    {
        int count = 0;
        int j;
        int dim_index = -1;
        int order[HARP_NUM_DIM_TYPES];

        var = product->variable[i];

        for (j = 0; j < var->num_dimensions; j++)
        {
            if (var->dimension_type[j] == dimension_type)
            {
                count++;
                dim_index = j;
            }
        }

        if (count == 0)
        {
            if (var->dimension_type[0] != harp_dimension_time)
            {
                /* skip variables that don't depend on the relevant dim AND not on time */
                continue;
            }
            else
            {
                /* add the dimension to be flattened in the right place;
                   this effectively extends time appropriately */
                if (harp_variable_add_dimension(var, 1, dimension_type, dim_length) != 0)
                {
                    return -1;
                }
                dim_index = 1;
                count = 1;
            }
        }
        else if (count >= 2)
        {
            /* remove variables that depend more than once on the specified dimension */
            harp_product_remove_variable(product, var);
            continue;
        }

        /* the variable must be time-dependend */
        if (var->dimension_type[0] != harp_dimension_time)
        {
            if (harp_variable_add_dimension(var, 0, harp_dimension_time, product->dimension[harp_dimension_time]))
            {
                return -1;
            }

            dim_index++;
        }

        /* derive the new order of dimensions, splicing in dim_index at position 1 *if necessary* */
        if (dim_index != 1)
        {
            order[0] = 0;
            order[1] = dim_index;
            for (j = 2; j < var->num_dimensions; j++)
            {
                if (j <= dim_index)
                {
                    order[j] = j - 1;
                }
                else
                {
                    order[j] = j;
                }
            }

            /* reorder dimensions */
            if (harp_array_transpose(var->data_type, var->num_dimensions, var->dimension, order, var->data) != 0)
            {
                return -1;
            }
        }

        /* update the dimension info */
        var->dimension[harp_dimension_time] *= var->dimension[dim_index];
        for (j = dim_index; j < var->num_dimensions - 1; j++)
        {
            var->dimension[j] = var->dimension[j + 1];
            var->dimension_type[j] = var->dimension_type[j + 1];
        }

        var->num_dimensions--;
    }

    /* update the dimension info of the product */
    product->dimension[harp_dimension_time] *= dim_length;
    product->dimension[dimension_type] = 0;

    return 0;
}