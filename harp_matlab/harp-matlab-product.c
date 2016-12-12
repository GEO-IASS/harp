/*
 * Copyright (C) 2002-2016 S[&]T, The Netherlands.
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

#include "harp-matlab.h"

#include <string.h>

#include "harp-utils.c"

static char *create_num_dims_variable(const char *variable_name)
{
    char *dim_variable_name;
    int length;

    length = strlen(variable_name);
    dim_variable_name = mxCalloc(length + 10, sizeof(char));
    strcpy(dim_variable_name, variable_name);
    strcpy(&dim_variable_name[length], "_num_dims");

    return dim_variable_name;
}

static int has_num_dims_extension(const char *variable_name)
{
    int length;
    length = strlen(variable_name);
    if (length > 9 && memcmp(&variable_name[length-9], "_num_dims", 9) == 0)  
    {       
        return 1;
    }

    return 0;
}

static void harp_matlab_add_harp_product_variable(mxArray *mx_struct, harp_product **product, int index)
{
    harp_variable *variable = (**product).variable[index];
    harp_data_type type = (*variable).data_type;
    const char *variable_name = (*variable).name;
    
    harp_array variable_data = (*variable).data;
   
    long dim[HARP_MAX_NUM_DIMS]; 
    harp_dimension_type dim_type[HARP_MAX_NUM_DIMS];// to add

    mwSize matlabdim[HARP_MAX_NUM_DIMS];
    mwSize matlabdimtype[HARP_MAX_NUM_DIMS];
    int num_dims = (*variable).num_dimensions;
    long num_elements= (*variable).num_elements;
    long i;
    mxArray *mx_data = NULL;
  
    if (harp_product_get_variable_by_name(*product, variable_name, &variable) != 0)
    {
        harp_matlab_harp_error();
    }
    if (harp_product_get_variable_id_by_name(*product, variable_name, &index) != 0)
    {
        harp_matlab_harp_error();
    }
    
    mxAssert(num_dims >= 0, "Number of dimensions is invalid");
    mxAssert(num_dims <= HARP_MAX_NUM_DIMS, "Number of dimensions is too high");
    mxAssert(num_elements > 0, "Number of elements in array is zero");


    for (i = 0; i < HARP_MAX_NUM_DIMS; i++){

         dim[i] = (*variable).dimension[i];
         dim_type[i] = (*variable).dimension_type[i];
    }


    /* Add extra _num_dims element if the last dimensions equals 1 */
    if (num_dims > 0 && dim[num_dims - 1] == 1)
    {

        // mexPrintf("---inside the add function--4- \n");  
        char *dim_variable_name;

        dim_variable_name = create_num_dims_variable(variable_name);
        mxAddField(mx_struct, dim_variable_name);
        mxSetField(mx_struct, 0, dim_variable_name, mxCreateDoubleScalar((double)num_dims));
        mxFree(dim_variable_name);
    }

    /* MATLAB does not allow creation of arrays with num_dims == 0 */
    if (num_dims == 0 && type != harp_type_string)
    {
        dim[num_dims++] = 1;
    }

    for (i = 0; i < num_dims; i++)
    {
            matlabdim[i] = (mwSize)dim[i];
            matlabdimtyep[i] = (mwSize) dim_type[i]
    }

    switch (type)
    {
        case harp_type_int8:
            {
                int8_t *data;
   
                mx_data = mxCreateNumericArray(num_dims, matlabdim, mxINT8_CLASS, mxREAL);
                data = mxGetData(mx_data);
                int counter = 0;
                while(counter<num_elements){
                for(mwSize j=0; j<num_elements/matlabdim[num_dims-1];j++){
                    for(mwSize k=0; k<matlabdim[num_dims-1];k++){
                        data[j+k*num_elements/matlabdim[num_dims-1]] = variable_data.int8_data[counter++];
                    } 
                 } 
                }

            }
            break;
        case harp_type_int16:
            {
                int16_t *data;
   
                mx_data = mxCreateNumericArray(num_dims, matlabdim, mxINT16_CLASS, mxREAL);
                data = mxGetData(mx_data);
                int counter = 0;
                while(counter<num_elements){
                for(mwSize j=0; j<num_elements/matlabdim[num_dims-1];j++){
                    for(mwSize k=0; k<matlabdim[num_dims-1];k++){
                        data[j+k*num_elements/matlabdim[num_dims-1]] = variable_data.int16_data[counter++];
                    } 
                 } 
                } 
            }
            break;    
        case harp_type_int32:
            {
                int32_t *data;    
              
                mx_data = mxCreateNumericArray(num_dims, matlabdim, mxINT32_CLASS, mxREAL);
                data = mxGetData(mx_data); 
                int counter = 0;
                while(counter<num_elements){
                for(mwSize j=0; j<num_elements/matlabdim[num_dims-1];j++){
                    for(mwSize k=0; k<matlabdim[num_dims-1];k++){
                        data[j+k*num_elements/matlabdim[num_dims-1]] = variable_data.int32_data[counter++];
                    } 
                 } 
                }
           }
            break;
        case harp_type_double:
            {
                    double *data;
                    mx_data = mxCreateNumericArray(num_dims, matlabdim, mxDOUBLE_CLASS, mxREAL);
                    data = mxGetData(mx_data);
                    int counter = 0;
                    while(counter<num_elements){
                    for(mwSize j=0; j<num_elements/matlabdim[num_dims-1];j++){
                        for(mwSize k=0; k<matlabdim[num_dims-1];k++){
                                 data[j+k*num_elements/matlabdim[num_dims-1]] = variable_data.double_data[counter++];
                                } 
                            } 
                    }                      
              
            }
            break;
        case harp_type_float:
            {
                float *data;

                mx_data = mxCreateNumericArray(num_dims, matlabdim, mxSINGLE_CLASS, mxREAL);
                data = mxGetData(mx_data);
                int counter = 0;
                while(counter<num_elements){
                for(mwSize j=0; j<num_elements/matlabdim[num_dims-1];j++){
                    for(mwSize k=0; k<matlabdim[num_dims-1];k++){
                        data[j+k*num_elements/matlabdim[num_dims-1]] = variable_data.float_data[counter++];
                    } 
                 } 
                }           

            }
            break; 
        case harp_type_string:
            if (num_dims == 0)
            {
                mx_data = mxCreateString(variable_data.string_data[0]);
            }
            else
            {
                mx_data = mxCreateCellArray(num_dims, matlabdim);
                for (i = 0; i < num_elements; i++)
                {
                    mxSetCell(mx_data, coda_c_index_to_fortran_index(num_dims, dim, i),
                              mxCreateString(variable_data.string_data[i]));
                }
            }
            break;
    }

    mxAddField(mx_struct, variable_name);
    mxSetField(mx_struct, 0, variable_name, mx_data);
}

mxArray *harp_matlab_get_product(harp_product **product)
{
    mxArray *mx_data = NULL;
    int num_variables;
    int index;

    num_variables = (**product).num_variables;
    if (num_variables == 1)
    {
        harp_matlab_harp_error();
    }
    mx_data = mxCreateStructMatrix(1, 1, 0, NULL);

    for (index = 0; index < num_variables; index++)
    {
        harp_matlab_add_harp_product_variable(mx_data, product, index);
    }

    return mx_data;
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


static void harp_matlab_add_matlab_product_variable(harp_product **product, const char *variable_name, mxArray *mx_variable,
                                                  int req_num_dims)
// static void harp_matlab_add_matlab_product_variable(harp_product **product, const char *variable_name, mxArray *mx_variable,
                                                  // const size_t *req_num_dims, int index_passing)
{
    mxClassID class;
   
    // mexPrintf("inside the add matlab product variable function");
    // mexPrintf("the name of the variable is: %s \n", variable_name );
    // mexPrintf("requested dim is: %d \n", req_num_dims );

    harp_variable *variable;

    char *string_data;
    long dim[HARP_MAX_NUM_DIMS];
    harp_dimension_type dim_type[HARP_MAX_NUM_DIMS];
    int num_dims;
    long num_elements;
    // int index;
    long i;

    class = mxGetClassID(mx_variable);
    num_dims = mxGetNumberOfDimensions(mx_variable);
    if (num_dims > HARP_MAX_NUM_DIMS)
    {
        mexErrMsgTxt("Number of dimensions for product variable is too high.");
    }
    for (i = 0; i < num_dims; i++)
    {
        dim[i] = (long)mxGetDimensions(mx_variable)[i];
        // dim_type[i] = harp_dimension_time;
    }
    dim_type[0] = harp_dimension_time;
    dim_type[1] = harp_dimension_independent;
    
   
    num_elements = mxGetNumberOfElements(mx_variable);
    // mexPrintf("-------value of the elements is--------- : %d \n", num_elements);
    // mexPrintf("-------dim 1--------- : %d \n", dim[0]);
    // mexPrintf("-------dim 2--------- : %d \n", dim[1]);
    //-- fix it later
    // if (num_elements == 0)
    // {
    //     if (class == mxCHAR_CLASS)
    //     {
    //         // index = harp_product_add_variable(product, variable);
    //         // if (index < 0)
    //         if(!(harp_product_add_variable(*product,variable)))
    //         {
    //             harp_matlab_harp_error();
    //         }
    //         return;
    //     }
    //     else
    //     {
    //         mexErrMsgTxt("Empty arrays are not allowed for a product variable.");
    //     }
    // }

    /* descrease number of dimensions to the lowest value possible */
    while (num_dims > 0 && dim[num_dims - 1] == 1)
    {
        num_dims--;
    }

    /* check if we need to increase the number of dimensions to a requested number of dimensions */
    if (req_num_dims >= 0 && req_num_dims <= HARP_MAX_NUM_DIMS)
    {
        if (req_num_dims < num_dims)
        {
            mexWarnMsgTxt("num_dims variable ignored. Its value was lower than the actual number of dimensions.");
        }
        else
        {
            while (num_dims < req_num_dims)
            {
                dim[num_dims++] = 1;
            }
        }
    }

    mxAssert(num_dims >= 0, "Number of dimensions is invalid");
    mxAssert(num_dims <= HARP_MAX_NUM_DIMS, "Number of dimensions is too high");

    switch (class)
    {
        case mxUINT8_CLASS:
            {
                int8_t *data;

                if( harp_variable_new(variable_name, harp_type_int8, num_dims, dim_type, dim, &variable) !=0)
                {
                    harp_matlab_harp_error();
                }
                if(harp_product_add_variable(*product, variable) != 0)
                {
                    harp_matlab_harp_error();
                }
                if(harp_product_get_variable_by_name(*product, variable_name, &variable)!=0)
                {
                    harp_matlab_harp_error();
                }
                data = mxGetData(mx_variable);
            
                int counter = 0;
                while(counter<num_elements){
                    for(long j=0; j<num_elements/dim[req_num_dims-1];j++){
                        for(long k=0; k<dim[req_num_dims-1];k++){
                            (*variable).data.int8_data[counter++] = data[j+k*num_elements/dim[req_num_dims-1]];
                        } 
                    } 
                }     
            }
            break;
        case mxUINT16_CLASS:
            {
                int16_t *data;

                if( harp_variable_new(variable_name, harp_type_int16, num_dims, dim_type, dim, &variable) !=0)
                {
                    harp_matlab_harp_error();
                }
                if( harp_product_add_variable(*product, variable)!= 0)
                {
                    harp_matlab_harp_error();
                }
                if(harp_product_get_variable_by_name(*product, variable_name, &variable)!=0)
                {
                    harp_matlab_harp_error();
                }
                int counter = 0;
                data = mxGetData(mx_variable);
                while(counter<num_elements){
                    for(long j=0; j<num_elements/dim[req_num_dims-1];j++){
                        for(long k=0; k<dim[req_num_dims-1];k++){
                            (*variable).data.int16_data[counter++] = data[j+k*num_elements/dim[req_num_dims-1]];
                        } 
                    } 
                }     
            }
            break;    
        case mxINT32_CLASS:
            {
                int32_t *data;
               
                if( harp_variable_new(variable_name, harp_type_int32, num_dims, dim_type, dim, &variable) !=0)
                {
                    harp_matlab_harp_error();
                }
                if( harp_product_add_variable(*product, variable) !=0)
                {
                    harp_matlab_harp_error();
                }
                if(harp_product_get_variable_by_name(*product, variable_name, &variable)!=0)
                {
                    harp_matlab_harp_error();
                }
                data = mxGetData(mx_variable);
                int counter = 0;
                while(counter<num_elements){
                    for(long j=0; j<num_elements/dim[req_num_dims-1];j++){
                        for(long k=0; k<dim[req_num_dims-1];k++){
                            (*variable).data.int32_data[counter++] = data[j+k*num_elements/dim[req_num_dims-1]];
                        } 
                    } 
                }  

            }
            break;
        case mxDOUBLE_CLASS:
            {
                double *data;
             
                if( harp_variable_new(variable_name, harp_type_double, num_dims, dim_type, dim, &variable) !=0)
                {
                    harp_matlab_harp_error();
                }
          
                if( harp_product_add_variable(*product, variable) !=0)    
                {
                    harp_matlab_harp_error();
                }
                mexPrintf("sucessfully added the variable \n");
                mexPrintf("verifying num of variables %d \n", (**product).num_variables);
            
                if(harp_product_get_variable_by_name(*product, variable_name, &variable)!=0)
                {
                    harp_matlab_harp_error();
                }

                data = mxGetData(mx_variable);
            
                int counter = 0;
                while(counter<num_elements){
                    for(long j=0; j<num_elements/dim[req_num_dims-1];j++){
                        for(long k=0; k<dim[req_num_dims-1];k++){
                            (*variable).data.double_data[counter++] = data[j+k*num_elements/dim[req_num_dims-1]];
                        } 
                    } 
                }    
                mexPrintf("finished one assigning of a variable");
            }
            break;
        case mxSINGLE_CLASS:
            {
                float *data;

                // index = harp_product_add_variable(product, variable);
                // if (index < 0)
                if( harp_variable_new(variable_name, harp_type_float, num_dims, dim_type, dim, &variable) !=0)
                {
                    harp_matlab_harp_error();
                }
                if( harp_product_add_variable(*product, variable)!= 0)
                {
                    harp_matlab_harp_error();
                }
                if(harp_product_get_variable_by_name(*product, variable_name, &variable)!=0)
                {
                    harp_matlab_harp_error();
                }
                data = mxGetData(mx_variable);
           
                int counter = 0;
                while(counter<num_elements){
                    for(long j=0; j<num_elements/dim[req_num_dims-1];j++){
                        for(long k=0; k<dim[req_num_dims-1];k++){
                            (*variable).data.float_data[counter++] = data[j+k*num_elements/dim[req_num_dims-1]];
                        } 
                    } 
                } 
            }     
            break;    
        case mxCHAR_CLASS:
        {
            if (mxGetNumberOfDimensions(mx_variable) != 2 || mxGetDimensions(mx_variable)[0] != 1)
            {
                mexErrMsgTxt("Multi-dimensional string arrays are not allowed. Use a cell array of strings instead.");
            }
            string_data = get_matlab_string_value(mx_variable);
          
            if( harp_variable_new(variable_name, harp_type_string, num_dims, dim_type, dim, &variable) !=0)
            {
                    harp_matlab_harp_error();
            }
            if( harp_product_add_variable(*product, variable)!=0)
             {
                    harp_matlab_harp_error();
             }

            mxFree(string_data);
            break;
        }
        case mxCELL_CLASS: // not sure
            {
                mxArray *mx_cell;

                for (i = 0; i < num_elements; i++)
                {
                    mx_cell = mxGetCell(mx_variable, i);
                    if (mxGetClassID(mx_cell) != mxCHAR_CLASS || mxGetNumberOfDimensions(mx_cell) != 2 ||
                        mxGetDimensions(mx_cell)[0] > 1)
                    {
                        mexErrMsgTxt("Cell arrays are only allowed for one dimensional string data.");
                    }
                }
                 if( harp_variable_new(variable_name, harp_type_string, num_dims, dim_type, dim, &variable) !=0)
                {
                    harp_matlab_harp_error();
                 }
                if( harp_product_add_variable(*product, variable)!=0)
                {
                    harp_matlab_harp_error();
                }
            
                for (i = 0; i < num_elements; i++)
                {
                    mx_cell = mxGetCell(mx_variable, coda_c_index_to_fortran_index(num_dims, dim, i));
                    string_data = get_matlab_string_value(mx_cell);
                    if (harp_variable_set_string_data_element(variable, i, string_data) != 0)
                    {
                        mxFree(string_data);
                        harp_matlab_harp_error();
                    }
                    mxFree(string_data);
                }
            }
            break;
        default:
            mexErrMsgTxt("Unsupported class for variabledata.");
            return;
    }
}

harp_product *harp_matlab_set_product(const mxArray *mx_struct)
{
    harp_product *product;
    // harp_product **newproduct;
    int num_variables;
    int index;

    if (!mxIsStruct(mx_struct))
    {
        mexErrMsgTxt("Not a struct.");
    }
    num_variables = mxGetNumberOfFields(mx_struct);

    if (harp_product_new(&product)!= 0)
    {
        harp_matlab_harp_error();
    }

    for (index = 0; index < num_variables; index++)
    {
        const char *variable_name;
        // mexPrintf(" number of the variables is: %d \n", num_variables);

        variable_name = mxGetFieldNameByNumber(mx_struct, index);
        // mexPrintf(" name of the variable is: %s \n", variable_name);
        if (!has_num_dims_extension(variable_name))
        {
            // const char *dim_variable_name;
            char *dim_variable_name;
            mxArray *mx_variable;
            mxArray *mx_dim_variable;
            int dim_variable_index;
            const size_t *dim2;
            int dim1=2;

            /* find and retrieve dimension variable if it exists */
            mx_dim_variable = NULL;
            dim_variable_name = create_num_dims_variable(variable_name);
            // dim_variable_name = variable_name;
            // dim_variable_index = mxGetFieldNumber(mx_struct, dim_variable_name);
            dim_variable_index = mxGetFieldNumber(mx_struct, variable_name);
            int number = mxGetNumberOfFields(mx_struct);
            // mexPrintf("number of the fields is: %d \n", number);
            // mexPrintf(" dim_variable_name is: %s \n", dim_variable_name);
            // mexPrintf(" dim_variable_index is: %d \n", dim_variable_index);
            mxFree(dim_variable_name);

            if (dim_variable_index >= 0)
            {   
                // mx_dim_variable = mxGetField(mx_struct,0, dim_variable_name);

                mx_dim_variable = mxGetFieldByNumber(mx_struct,0, dim_variable_index);
                int elements = mxGetNumberOfElements(mx_dim_variable);
                dim1 = mxGetNumberOfDimensions(mx_dim_variable);
                dim2 = mxGetDimensions(mx_dim_variable);
                // mexPrintf("dimension 1 of the variable is: %d \n", dim1);
            }

            mx_variable = mxGetFieldByNumber(mx_struct, 0, index);
    
            harp_matlab_add_matlab_product_variable(&product, variable_name, mx_variable, dim1);
    
        }
    }

    return product;
}
