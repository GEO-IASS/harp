/*
 * Copyright (C) 2002-2016 S[&]T, The Netherlands.
 *
 * This file is part of BEAT.
 *
 * BEAT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * BEAT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BEAT; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "harp-matlab.h"

#include <string.h>

#include "harp-utils.h"

static char *create_num_dims_field(const char *field_name)
{
    char *dim_field_name;
    int length;

    length = strlen(field_name);
    dim_field_name = mxCalloc(length + 10, sizeof(char));
    strcpy(dim_field_name, field_name);
    strcpy(&dim_field_name[length], "_num_dims");

    return dim_field_name;
}

static int has_num_dims_extension(const char *field_name)
{
    int length;

    length = strlen(field_name);
    if (length > 9 && memcmp(&field_name[length - 9], "_num_dims", 9) == 0)
    {
        return 1;
    }

    return 0;
}

// static void harp_matlab_add_harp_product_field(mxArray *mx_struct, harp_product *product, int index)
// {
//     harp_BasicType type;
//     mxArray *mx_data = NULL;
//     const char *field_name;
//     harp_DataType field_data;
//     long dim[harp_MAX_NUM_DIMS];
//     mwSize matlabdim[harp_MAX_NUM_DIMS];
//     int num_dims;
//     long num_elements;
//     long i;

//     if (harp_product_get_field_name(record, index, &field_name) != 0)
//     {
//         harp_matlab_harp_error();
//     }
//     if (harp_product_get_field_type(record, index, &type) != 0)
//     {
//         harp_matlab_harp_error();
//     }
//     if (harp_product_get_field_dim(record, index, &num_dims, dim) != 0)
//     {
//         harp_matlab_harp_error();
//     }
//     if (harp_product_get_field_num_elements(record, index, &num_elements) != 0)
//     {
//         harp_matlab_harp_error();
//     }
//     if (harp_product_get_field_data(record, index, &field_data) != 0)
//     {
//         harp_matlab_harp_error();
//     }

//     mxAssert(num_dims >= 0, "Number of dimensions is invalid");
//     mxAssert(num_dims <= harp_MAX_NUM_DIMS, "Number of dimensions is too high");
//     mxAssert(num_elements > 0, "Number of elements in array is zero");

//     /* Add extra _num_dims element if the last dimensions equals 1 */
//     if (num_dims > 0 && dim[num_dims - 1] == 1)
//     {
//         char *dim_field_name;

//         dim_field_name = create_num_dims_field(field_name);
//         mxAddField(mx_struct, dim_field_name);
//         mxSetField(mx_struct, 0, dim_field_name, mxCreateDoubleScalar((double)num_dims));
//         mxFree(dim_field_name);
//     }

//     /* MATLAB does not allow creation of arrays with num_dims == 0 */
//     if (num_dims == 0 && type != harp_string_ptr)
//     {
//         dim[num_dims++] = 1;
//     }

//     for (i = 0; i < num_dims; i++)
//     {
//         matlabdim[i] = (mwSize)dim[i];
//     }

//     switch (type)
//     {
//         case harp_uint8:
//             {
//                 uint8_t *data;

//                 mx_data = mxCreateNumericArray(num_dims, matlabdim, mxINT32_CLASS, mxREAL);
//                 data = mxGetData(mx_data);
//                 harp_c_array_to_fortran_array_uint8(field_data.uint8_data, data, num_dims, dim);
//             }
//             break;
//         case harp_int32:
//             {
//                 int32_t *data;

//                 mx_data = mxCreateNumericArray(num_dims, matlabdim, mxINT32_CLASS, mxREAL);
//                 data = mxGetData(mx_data);
//                 harp_c_array_to_fortran_array_int32(field_data.int32_data, data, num_dims, dim);
//             }
//             break;
//         case harp_double:
//             {
//                 double *data;

//                 mx_data = mxCreateNumericArray(num_dims, matlabdim, mxDOUBLE_CLASS, mxREAL);
//                 data = mxGetData(mx_data);
//                 harp_c_array_to_fortran_array_double(field_data.double_data, data, num_dims, dim);
//             }
//             break;
//         case harp_string_ptr:
//             if (num_dims == 0)
//             {
//                 mx_data = mxCreateString(field_data.string_data[0]);
//             }
//             else
//             {
//                 mx_data = mxCreateCellArray(num_dims, matlabdim);
//                 for (i = 0; i < num_elements; i++)
//                 {
//                     mxSetCell(mx_data, coda_c_index_to_fortran_index(num_dims, dim, i),
//                               mxCreateString(field_data.string_data[i]));
//                 }
//             }
//             break;
//     }

//     mxAddField(mx_struct, field_name);
//     mxSetField(mx_struct, 0, field_name, mx_data);
// }

mxArray *harp_matlab_get_record(harp_product *product)
{
    mxArray *mx_data = NULL;
    int num_fields;
    int index;

    num_fields = harp_product_get_num_fields(product);
    if (num_fields < 0)
    {
        harp_matlab_harp_error();
    }
    mx_data = mxCreateStructMatrix(1, 1, 0, NULL);

    for (index = 0; index < num_fields; index++)
    {
        harp_matlab_add_harp_product_field(mx_data, product, index);
    }

    return mx_data;
}

static int get_mx_dim_field_value(mxArray *mx_dim_field)
{
    if (mx_dim_field == NULL)
    {
        return -1;
    }

    if (mxIsDouble(mx_dim_field) && mxGetNumberOfElements(mx_dim_field) == 1)
    {
        int dim_value;

        dim_value = (int)mxGetScalar(mx_dim_field);
        if (dim_value < 0)
        {
            mexWarnMsgTxt("num_dims field ignored. Its value was negative.");
            return -1;
        }
        // if (dim_value > harp_MAX_NUM_DIMS)
        // {
        //     mexWarnMsgTxt("num_dims field ignored. Its value was too high.");
        //     return -1;
        // }

        return dim_value;
    }

    mexErrMsgTxt("Invalid num_dims field found.");

    return -1;
}

static char *get_matlab_string_value(mxArray *mx_data)
{
    char *string_data;
    int buflen;

    buflen = (mxGetNumberOfElements(mx_data) * sizeof(mxChar)) + 1;
    string_data = mxCalloc(buflen, 1);
    mxGetString(mx_data, string_data, buflen);

    return string_data;
}


// static void harp_matlab_add_matlab_record_field(harp_product **product, const char *field_name, mxArray *mx_field,
//                                                   int req_num_dims)
// {
//     mxClassID class;
//     harp_DataType field_data;
//     char *string_data;
//     long dim[harp_MAX_NUM_DIMS];
//     int num_dims;
//     long num_elements;
//     int index;
//     long i;

//     class = mxGetClassID(mx_field);
//     num_dims = mxGetNumberOfDimensions(mx_field);
//     if (num_dims > harp_MAX_NUM_DIMS)
//     {
//         mexErrMsgTxt("Number of dimensions for record field is too high.");
//     }
//     for (i = 0; i < num_dims; i++)
//     {
//         dim[i] = (long)mxGetDimensions(mx_field)[i];
//     }
//     num_elements = mxGetNumberOfElements(mx_field);
//     if (num_elements == 0)
//     {
//         if (class == mxCHAR_CLASS)
//         {
//             index = harp_product_add_string_field(*product, field_name, "");
//             if (index < 0)
//             {
//                 harp_matlab_harp_error();
//             }
//             return;
//         }
//         else
//         {
//             mexErrMsgTxt("Empty arrays are not allowed for a record field.");
//         }
//     }

//     /* descrease number of dimensions to the lowest value possible */
//     while (num_dims > 0 && dim[num_dims - 1] == 1)
//     {
//         num_dims--;
//     }

//     /* check if we need to increase the number of dimensions to a requested number of dimensions */
//     if (req_num_dims >= 0 && req_num_dims <= harp_MAX_NUM_DIMS)
//     {
//         if (req_num_dims < num_dims)
//         {
//             mexWarnMsgTxt("num_dims field ignored. Its value was lower than the actual number of dimensions.");
//         }
//         else
//         {
//             while (num_dims < req_num_dims)
//             {
//                 dim[num_dims++] = 1;
//             }
//         }
//     }

//     mxAssert(num_dims >= 0, "Number of dimensions is invalid");
//     mxAssert(num_dims <= harp_MAX_NUM_DIMS, "Number of dimensions is too high");

//     switch (class)
//     {
//         case mxUINT8_CLASS:
//             {
//                 uint8_t *data;

//                 index = harp_product_add_field(*product, field_name, harp_uint8, num_dims, dim);
//                 if (index < 0)
//                 {
//                     harp_matlab_harp_error();
//                 }
//                 if (harp_product_get_field_data(*product, index, &field_data) != 0)
//                 {
//                     harp_matlab_harp_error();
//                 }
//                 data = mxGetData(mx_field);
//                 harp_fortran_array_to_c_array_uint8(data, field_data.uint8_data, num_dims, dim);
//             }
//             break;
//         case mxINT32_CLASS:
//             {
//                 int32_t *data;

//                 index = harp_product_add_field(*product, field_name, harp_int32, num_dims, dim);
//                 if (index < 0)
//                 {
//                     harp_matlab_harp_error();
//                 }
//                 if (harp_product_get_field_data(*product, index, &field_data) != 0)
//                 {
//                     harp_matlab_harp_error();
//                 }
//                 data = mxGetData(mx_field);
//                 harp_fortran_array_to_c_array_int32(data, field_data.int32_data, num_dims, dim);
//             }
//             break;
//         case mxDOUBLE_CLASS:
//             {
//                 double *data;

//                 index = harp_product_add_field(*product, field_name, harp_double, num_dims, dim);
//                 if (index < 0)
//                 {
//                     harp_matlab_harp_error();
//                 }
//                 if (harp_product_get_field_data(*product, index, &field_data) != 0)
//                 {
//                     harp_matlab_harp_error();
//                 }
//                 data = mxGetData(mx_field);
//                 harp_fortran_array_to_c_array_double(data, field_data.double_data, num_dims, dim);
//             }
//             break;
//         case mxCHAR_CLASS:
//             if (mxGetNumberOfDimensions(mx_field) != 2 || mxGetDimensions(mx_field)[0] != 1)
//             {
//                 mexErrMsgTxt("Multi-dimensional string arrays are not allowed. Use a cell array of strings instead.");
//             }
//             string_data = get_matlab_string_value(mx_field);
//             index = harp_product_add_string_field(*product, field_name, string_data);
//             if (index < 0)
//             {
//                 harp_matlab_harp_error();
//             }
//             mxFree(string_data);
//             break;
//         case mxCELL_CLASS:
//             {
//                 mxArray *mx_cell;

//                 for (i = 0; i < num_elements; i++)
//                 {
//                     mx_cell = mxGetCell(mx_field, i);
//                     if (mxGetClassID(mx_cell) != mxCHAR_CLASS || mxGetNumberOfDimensions(mx_cell) != 2 ||
//                         mxGetDimensions(mx_cell)[0] > 1)
//                     {
//                         mexErrMsgTxt("Cell arrays are only allowed for one dimensional string data.");
//                     }
//                 }
//                 index = harp_product_add_field(*product, field_name, harp_string_ptr, num_dims, dim);
//                 if (index < 0)
//                 {
//                     harp_matlab_harp_error();
//                 }
//                 if (harp_product_get_field_data(*product, index, &field_data) != 0)
//                 {
//                     harp_matlab_harp_error();
//                 }
//                 for (i = 0; i < num_elements; i++)
//                 {
//                     mx_cell = mxGetCell(mx_field, coda_c_index_to_fortran_index(num_dims, dim, i));
//                     string_data = get_matlab_string_value(mx_cell);
//                     if (harp_product_set_string_data_element(field_data, i, string_data) != 0)
//                     {
//                         mxFree(string_data);
//                         harp_matlab_harp_error();
//                     }
//                     mxFree(string_data);
//                 }
//             }
//             break;
//         default:
//             mexErrMsgTxt("Unsupported class for fielddata.");
//             return;
//     }
// }

// harp_product *harp_matlab_set_record(const mxArray *mx_struct)
// {
//     harp_product *product;
//     int num_fields;
//     int index;

//     if (!mxIsStruct(mx_struct))
//     {
//         mexErrMsgTxt("Not a struct.");
//     }
//     num_fields = mxGetNumberOfFields(mx_struct);

//     record = harp_product_create();
//     if (record == NULL)
//     {
//         harp_matlab_harp_error();
//     }

//     for (index = 0; index < num_fields; index++)
//     {
//         const char *field_name;

//         field_name = mxGetFieldNameByNumber(mx_struct, index);
//         if (!has_num_dims_extension(field_name))
//         {
//             char *dim_field_name;
//             mxArray *mx_field;
//             mxArray *mx_dim_field;
//             int dim_field_index;

//             /* find and retrieve dimension field if it exists */
//             mx_dim_field = NULL;
//             dim_field_name = create_num_dims_field(field_name);
//             dim_field_index = mxGetFieldNumber(mx_struct, dim_field_name);
//             mxFree(dim_field_name);
//             if (dim_field_index >= 0)
//             {
//                 mx_dim_field = mxGetFieldByNumber(mx_struct, 0, dim_field_index);
//             }

//             mx_field = mxGetFieldByNumber(mx_struct, 0, index);

//             harp_matlab_add_matlab_record_field(&record, mxGetFieldNameByNumber(mx_struct, index), mx_field,
//                                                   get_mx_dim_field_value(mx_dim_field));
//         }
//     }

//     return record;
// }
