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
    // mexPrintf("--here 1---\n");    
    harp_variable *variable = (**product).variable[index];
    harp_data_type type = (*variable).data_type;
    const char *variable_name = (*variable).name;
    harp_array variable_data = (*variable).data;
  
      /**--more variables info--**/
    char * description = (*variable).description;
    char * unit = (*variable).unit;

    long dim[HARP_MAX_NUM_DIMS]; 
    harp_dimension_type dim_type[HARP_MAX_NUM_DIMS];

    mwSize matlabdim[HARP_MAX_NUM_DIMS];
    mwSize matlabdim_type[HARP_MAX_NUM_DIMS];
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
        if(dim[i]>1){
            dim_type[i] = (*variable).dimension_type[i];
        }
    }

    // top-level
    mxArray* struct_data = mxCreateStructMatrix(1,1,0, NULL);
    
    // mexPrintf("number of dimensions:%d \n", num_dims);
    /**--add more infomation for each variable--**/    
    // mexPrintf("---before adding the fields---\n");
   
        mxArray * string_des = mxCreateString(description);
        mxAddField(struct_data, "description");
    if(description != NULL){    
        mxSetField(struct_data, 0, "description", string_des);
    }
    
        mxArray * string_unit = mxCreateString(unit);    
        mxAddField(struct_data, "unit");
    if(unit != NULL){    
        mxSetField(struct_data, 0, "unit", string_unit);
    }


    //------- also to deal when the last dimension equals to 1 --------
    /* Add extra _num_dims element if the last dimensions equals 1 */
    // if (num_dims > 0 && dim[num_dims - 1] == 1)
    // {
    //     char *dim_variable_name;
    //     dim_variable_name = create_num_dims_variable(variable_name);
    //     mxAddField(mx_struct, dim_variable_name);
    //     mxSetField(mx_struct, 0, dim_variable_name, mxCreateDoubleScalar((double)num_dims));
    //     mxFree(dim_variable_name);
    // }

    /* MATLAB does not allow creation of arrays with num_dims == 0 */
    if (num_dims == 0 && type != harp_type_string)
    {
        dim[num_dims++] = 1;
    }

    // added: dim_type and dim

    for (i = 0; i < num_dims; i++)
    {
        matlabdim[i] = (mwSize)dim[i];    
        // matlabdim_type[i] = (mwSize)dim_type[i];
        // mexPrintf("types of dimensions are:%d \n", dim_type[i]);
    }


    matlabdim_type[0] = num_dims;
    for (i = 1; i < num_dims; i++)
    {
        matlabdim_type[i] = 0;
    }
  
   

    mxArray * dim_info = mxCreateNumericArray(1,matlabdim_type,mxINT32_CLASS,mxREAL);  
    int *data1 = mxGetData(dim_info);

    mxArray * dim_info_type = mxCreateNumericArray(1,matlabdim_type,mxINT32_CLASS,mxREAL);    
    int *data2 = mxGetData(dim_info_type);
    // mexPrintf("must be here -before\n");
    for (i = 0; i < num_dims; i++)
    {   
            // mexPrintf("i was only: %d\n", i); 
            // data1[i] =  dim_type[i]; ;
            data1[i] = dim[i];
            data2[i] = dim_type[i];
            // mexPrintf("just want to know the values, i is: %d \n", i);
            // mexPrintf("dim info is: %d \n", dim_type[i]);
    }

    // mexPrintf("must be here -after\n");
    mxAddField(struct_data, "dimension");
    mxSetField(struct_data, 0, "dimension", dim_info);
   
    mxAddField(struct_data, "dimension_type");
    mxSetField(struct_data, 0, "dimension_type", dim_info_type);

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

 

    mxAddField(struct_data, "value");
    mxSetField(struct_data, 0, "value", mx_data);


    //back to the top-level again
    mxAddField(mx_struct, variable_name);
    mxSetField(mx_struct, 0, variable_name, struct_data);
}

mxArray *harp_matlab_get_product(harp_product **product)
{
    mxArray *mx_data = NULL;
    int num_variables;
    int index;
    // int fieldnum;
    char * source_product = (**product).source_product;
    char * history = (**product).history;



    num_variables = (**product).num_variables;
    if (num_variables == 1)
    {
        harp_matlab_harp_error();
    }
    mx_data = mxCreateStructMatrix(1, 1, 0, NULL);


    /**-----add meta data--------**/
    mxArray * string_source = mxCreateString(source_product);
    mxArray * string_his = mxCreateString(history);
  

    mxAddField(mx_data, "source");
    mxSetField(mx_data, 0, "source", string_source);
    // fieldnum =0;

    
    mxAddField(mx_data, "history");
    if(history != NULL){
        mxSetField(mx_data, 1, "history", string_his);
        // fieldnum ++;
    }
    
    /**----add variables-----**/
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
// static void harp_matlab_add_matlab_product_variable(harp_product **product, char *variable_name, mxArray *mx_variable,
//                                                   int req_num_dims)
{
    mxClassID class;
   
    mexPrintf("---inside the add matlab product variable function---");
    mexPrintf("the name of the variable is: %s \n", variable_name );
    // mexPrintf("requested dim is: %d \n", req_num_dims );

    // harp_variable *variable;
    harp_variable *variable_new;

    char *string_data;
    long dim[HARP_MAX_NUM_DIMS];
    harp_dimension_type dim_type[HARP_MAX_NUM_DIMS];
    int num_dims;
    long num_elements;
    // int index;
    long i;

    /*---get top level from matlab---*/
    int iindex;

    if (!mxIsStruct(mx_variable))
    {
        mexErrMsgTxt("This variable is not a struct.");
    }
    int num_fields = mxGetNumberOfFields(mx_variable);
    mexPrintf("the loops are: %d \n", num_fields);

    char * des_string;
    char * unit_string;
    int32_t *dimtypevalue;
    int32_t *dimvalue;

    for (iindex = 0; iindex < num_fields; iindex++)
    {
        const char *field_name;
      

        field_name = mxGetFieldNameByNumber(mx_variable, iindex);
       
        /*------set meta info-------*/
        if(strncmp(field_name,"description",11) ==0){
            mxArray * meta_variable  = mxGetField(mx_variable, 0, field_name);
            des_string = mxArrayToString(meta_variable);
            mexPrintf("description is %s \n", des_string);
            // variable_new->description = des_string;
        }
        else if(strncmp(field_name,"unit",4)==0){
            mxArray * meta_variable  = mxGetField(mx_variable, 0, field_name);
            unit_string = mxArrayToString(meta_variable);
            // variable_new->unit = unit_string;
        }
        
        else if(strncmp(field_name,"dimensions",10)==0){
            mxArray * meta_variable  = mxGetField(mx_variable, 0, field_name);
            // int num_dims_variable = mxGetNumberOfDimensions(meta_variable);
            dimvalue = mxGetData(meta_variable);
        }
        else if(strncmp(field_name,"dimension_type",14)==0){
            mxArray * meta_variable  = mxGetField(mx_variable, 0, field_name);
            int num_dims_variable = mxGetNumberOfDimensions(meta_variable);
            dimtypevalue = mxGetData(meta_variable);
            mexPrintf("num_dims of dim type is: %d \n", num_dims_variable);     
        }
        else if(strncmp(field_name,"value",5)==0)
        {
            mexPrintf("||||||name passed is: %s \n", variable_name);
            // variable->name =variable_name;

            mxArray * datastructure = mxGetField(mx_variable, 0, field_name);;    
            // mexPrintf("read IN the real values! \n" );
            // mexPrintf("field name is: %s \n", field_name);
            class = mxGetClassID(datastructure);
            num_dims = mxGetNumberOfDimensions(datastructure);

            if (num_dims > HARP_MAX_NUM_DIMS)
            {
                mexErrMsgTxt("Number of dimensions for product variable is too high.");
            }
            for (i = 0; i < num_dims; i++)
            {
                dim[i] = (long)mxGetDimensions(datastructure)[i];
                if(dim[i]>1){
                 // dim_type[i] = variable_new->dimension_type[i];
                        // dim_type[i] = dimtypevalue[i];
                    switch(dimtypevalue[i])
                    {
                      case -1:      
                      {
                        dim_type[i] = harp_dimension_independent;
                      } break;   
                      case 0:      
                      {
                        dim_type[i] = harp_dimension_time;
                      } break; 
                      case 1:      
                      {
                        dim_type[i] = harp_dimension_latitude;
                      }  break; 
                      case 2:      
                      {
                        dim_type[i] = harp_dimension_longitude;
                      } break; 
                      case 3:      
                      {
                        dim_type[i] = harp_dimension_vertical;
                      }  break; 
                      case 4:      
                      {
                        dim_type[i] = harp_dimension_spectral;
                      } break; 
                    } 
             

                     mexPrintf("type dimensions is %d \n",dim_type[i]);
                    }
               
            }

            // dim_type[0] = harp_dimension_time;
            // dim_type[1] = harp_dimension_independent;

            num_elements = mxGetNumberOfElements(datastructure);
            // mexPrintf("-------number of the elements is--------- : %d \n", num_elements);
            // mexPrintf("-------dim 1--------- : %d \n", dim[0]);
            // mexPrintf("-------dim 2--------- : %d \n", dim[1]);
            // mexPrintf("-------dim type 1--------- : %d \n", dim_type[0]);
            // if(dim[1]>1){
                // mexPrintf("-------dim type 2--------- : %d \n", dim_type[1]);
            // }else{
                // mexPrintf("there is no second dimension \n");
            // }
            //-- fix it later
            if (num_elements == 0)
            {
                if (class == mxCHAR_CLASS)
                {
                    // index = harp_product_add_variable(product, variable);
                    // if (index < 0)
                    if(!(harp_product_add_variable(*product,variable_new)))
                    {
                        harp_matlab_harp_error();
                    }
                    return;
                }
                else
                {
                    mexErrMsgTxt("Empty arrays are not allowed for a product variable.");
                }
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

            /* descrease number of dimensions to the lowest value possible */
            while (num_dims > 0 && dim[num_dims - 1] == 1)
            {
                mexPrintf("did go inside here \n");
                num_dims--;
            }


            mxAssert(num_dims >= 0, "Number of dimensions is invalid");
            mxAssert(num_dims <= HARP_MAX_NUM_DIMS, "Number of dimensions is too high");
            mexPrintf("num_dims afterwards is: %d \n", num_dims);
            switch (class)
            {
                case mxUINT8_CLASS:
                    {
                        int8_t *data;

                        if( harp_variable_new(variable_name, harp_type_int8, num_dims, dim_type, dim, &variable_new) !=0)
                        {
                            harp_matlab_harp_error();
                        }
                        if(harp_product_add_variable(*product, variable_new) != 0)
                        {
                            harp_matlab_harp_error();
                        }
                        if(harp_product_get_variable_by_name(*product, variable_name, &variable_new)!=0)
                        {
                            harp_matlab_harp_error();
                        }
                        data = mxGetData(datastructure);
                        int inner = (**product).num_variables;

                        // assigning meta data
                        (**product).variable[inner-1]->unit = unit_string;
                        (**product).variable[inner-1]->description = des_string;
                        // mxFree(unit_string);   
                        // mxFree(des_string);   

                        int counter = 0;
                        while(counter<num_elements){
                            for(long j=0; j<num_elements/dim[req_num_dims-1];j++){
                                for(long k=0; k<dim[req_num_dims-1];k++){
                                    (*variable_new).data.int8_data[counter++] = data[j+k*num_elements/dim[num_dims-1]];
                                } 
                            } 
                        }     
                    }
                    break;
                case mxUINT16_CLASS:
                    {
                        int16_t *data;

                        if( harp_variable_new(variable_name, harp_type_int16, num_dims, dim_type, dim, &variable_new) !=0)
                        {
                            harp_matlab_harp_error();
                        }
                        if( harp_product_add_variable(*product, variable_new)!= 0)
                        {
                            harp_matlab_harp_error();
                        }
                        if(harp_product_get_variable_by_name(*product, variable_name, &variable_new)!=0)
                        {
                            harp_matlab_harp_error();
                        }
                        
                        int inner = (**product).num_variables;

                        // assigning meta data
                        (**product).variable[inner-1]->unit = unit_string;
                        (**product).variable[inner-1]->description = des_string;
                        // mxFree(unit_string);   
                        // mxFree(des_string);   

                        data = mxGetData(datastructure);
                        int counter = 0;
                        while(counter<num_elements){
                            for(long j=0; j<num_elements/dim[req_num_dims-1];j++){
                                for(long k=0; k<dim[req_num_dims-1];k++){
                                    (*variable_new).data.int16_data[counter++] = data[j+k*num_elements/dim[num_dims-1]];
                                } 
                            } 
                        }     
                    }
                    break;    
                case mxINT32_CLASS:
                    {
                        int32_t *data;
                       
                        if( harp_variable_new(variable_name, harp_type_int32, num_dims, dim_type, dim, &variable_new) !=0)
                        {
                            harp_matlab_harp_error();
                        }
                        if( harp_product_add_variable(*product, variable_new) !=0)
                        {
                            harp_matlab_harp_error();
                        }
                        if(harp_product_get_variable_by_name(*product, variable_name, &variable_new)!=0)
                        {
                            harp_matlab_harp_error();
                        }
                        // mexPrintf("sucessfully added the INT32 variable \n");
                        // mexPrintf("verifying no of variables %d \n", (**product).num_variables);
                        int inner = (**product).num_variables;

                        // assigning meta data
                        (**product).variable[inner-1]->unit = unit_string;
                        (**product).variable[inner-1]->description = des_string;
                        // mxFree(unit_string);   
                        // mxFree(des_string);   

                        data = mxGetData(datastructure);
                        // mexPrintf("before the loop, the integerals is: %d \n", data[0]);
                        int counter = 0;
                        while(counter<num_elements){
                            for(long j=0; j<num_elements/dim[req_num_dims-1];j++){
                                for(long k=0; k<dim[req_num_dims-1];k++){
                                    variable_new->data.int32_data[counter++] = data[j+k*num_elements/dim[num_dims-1]];
                                } 
                            } 
                        }  
                        // mexPrintf("finished assigning of the last one! \n");

                    }
                    break;
                case mxDOUBLE_CLASS:
                    {
                        double *data;
                     
                        if( harp_variable_new(variable_name, harp_type_double, num_dims, dim_type, dim, &variable_new) !=0)
                        {
                            harp_matlab_harp_error();
                        }
                  
                        if( harp_product_add_variable(*product, variable_new) !=0)    
                        {
                            harp_matlab_harp_error();
                        }
                                            
                        if(harp_product_get_variable_by_name(*product, variable_name, &variable_new)!=0)
                        {
                            harp_matlab_harp_error();
                        }

                        mexPrintf("sucessfully added the DOUBLE variable \n");
                        mexPrintf("verifying num of variables %d \n", (**product).num_variables);
                        int inner = (**product).num_variables;

                        // assigning meta data
                        (**product).variable[inner-1]->unit = unit_string;
                        (**product).variable[inner-1]->description = des_string;
                        // mxFree(unit_string);   
                        // mxFree(des_string);   
                        // mexPrintf("dimensions number is: %d \n", num_dims);
                        // mexPrintf("dimensions is: %d \n", (**product).variable[inner-1]->num_dimensions);
                        
                        if(mxIsDouble(datastructure)){
                            mexPrintf("this is actually DOUBLE TYPE \n");
                        }
                   
                        data = mxGetPr(datastructure);
                        int counter = 0;
                        while(counter<num_elements){
                            for(long j=0; j<num_elements/dim[req_num_dims-1];j++){
                                for(long k=0; k<dim[req_num_dims-1];k++){
                                    variable_new->data.double_data[counter++] = data[j+k*num_elements/dim[num_dims-1]];
                                } 
                            } 
                        }    

                        
                            // mexPrintf("------$$$$$$---variable name is---: %s \n", (**product).variable[inner-1]->name);
                            // mexPrintf("--- name is---: %s \n", variable_name);
                            // mexPrintf("--- index is---: %d \n", inner-1);
                            // mexPrintf("---unit is---- %s \n", (**product).variable[inner-1]->unit);
                            // mexPrintf("---description is---- %s \n", (**product).variable[inner-1]->description);
                            // mexPrintf("dimension type 1 is: %d \n", (**product).variable[inner-1]->dimension_type[0]);
                        // if(num_dims==2){       
                            // mexPrintf("dimension type 2 is: %d \n", (**product).variable[inner-1]->dimension_type[1]);
                        // }   

                  
                        // data = mxGetPr(datastructure);
                        // if ( harp_array_transpose(harp_type_double,  (**product).variable[inner-1]->num_dimensions, const long *dimension, const int *order, harp_array data) !=0)
                        // {
                        //      harp_matlab_harp_error();
                        // }

                        mexPrintf("finished one assigning of a variable \n");
                    }
                    break;
                case mxSINGLE_CLASS:
                    {
                        float *data;

                        // index = harp_product_add_variable(product, variable);
                        // if (index < 0)
                        if( harp_variable_new(variable_name, harp_type_float, num_dims, dim_type, dim, &variable_new) !=0)
                        {
                            harp_matlab_harp_error();
                        }
                        if( harp_product_add_variable(*product, variable_new)!= 0)
                        {
                            harp_matlab_harp_error();
                        }
                        if(harp_product_get_variable_by_name(*product, variable_name, &variable_new)!=0)
                        {
                            harp_matlab_harp_error();
                        }
                        data = mxGetData(datastructure);
                        int inner = (**product).num_variables;

                        // assigning meta data
                        (**product).variable[inner-1]->unit = unit_string;
                        (**product).variable[inner-1]->description = des_string;
                        mxFree(unit_string);   
                        mxFree(des_string);   
                   
                        int counter = 0;
                        while(counter<num_elements){
                            for(long j=0; j<num_elements/dim[req_num_dims-1];j++){
                                for(long k=0; k<dim[req_num_dims-1];k++){
                                    (*variable_new).data.float_data[counter++] = data[j+k*num_elements/dim[num_dims-1]];
                                } 
                            } 
                        } 
                    }     
                    break;    
                case mxCHAR_CLASS:
                {
                    if (mxGetNumberOfDimensions(datastructure) != 2 || mxGetDimensions(datastructure)[0] != 1)
                    {
                        mexErrMsgTxt("Multi-dimensional string arrays are not allowed. Use a cell array of strings instead.");
                    }
                    string_data = get_matlab_string_value(datastructure);
                  
                    if( harp_variable_new(variable_name, harp_type_string, num_dims, dim_type, dim, &variable_new) !=0)
                    {
                            harp_matlab_harp_error();
                    }
                    if( harp_product_add_variable(*product, variable_new)!=0)
                     {
                            harp_matlab_harp_error();
                     }
                     int inner = (**product).num_variables;

                     // assigning meta data
                     (**product).variable[inner-1]->unit = unit_string;
                     (**product).variable[inner-1]->description = des_string;
                     // mxFree(unit_string);   
                     // mxFree(des_string);   

                    mxFree(string_data);
                    break;
                }
                case mxCELL_CLASS: // not sure
                    {
                        mxArray *mx_cell;

                        for (i = 0; i < num_elements; i++)
                        {
                            mx_cell = mxGetCell(datastructure, i);
                            if (mxGetClassID(mx_cell) != mxCHAR_CLASS || mxGetNumberOfDimensions(mx_cell) != 2 ||
                                mxGetDimensions(mx_cell)[0] > 1)
                            {
                                mexErrMsgTxt("Cell arrays are only allowed for one dimensional string data.");
                            }
                        }
                         if( harp_variable_new(variable_name, harp_type_string, num_dims, dim_type, dim, &variable_new) !=0)
                        {
                            harp_matlab_harp_error();
                         }
                        if( harp_product_add_variable(*product, variable_new)!=0)
                        {
                            harp_matlab_harp_error();
                        }

                        mexPrintf("~~~~CELL array~~~1 \n");
                        int inner = (**product).num_variables;

                        mexPrintf("~~~~CELL array~~~2 \n");
                        // assigning meta data
                        if(unit_string != NULL){
                            (**product).variable[inner-1]->unit = unit_string;
                        }
                        mexPrintf("~~~~CELL array~~~3 \n");
                        if(des_string != NULL){
                            (**product).variable[inner-1]->description = des_string;
                        }    
                        mexPrintf("~~~~CELL array~~~4 \n");
                        

                        mexPrintf("~~~~CELL array~~~5 \n");
                    
                        for (i = 0; i < num_elements; i++)
                        {
                            mx_cell = mxGetCell(datastructure, coda_c_index_to_fortran_index(num_dims, dim, i));
                            string_data = get_matlab_string_value(mx_cell);
                            if (harp_variable_set_string_data_element(variable_new, i, string_data) != 0)
                            {
                                mxFree(string_data);
                                harp_matlab_harp_error();
                            }
                            mxFree(string_data);
                        }

                        // mxFree(unit_string);   
                        // mxFree(des_string);   
                    }
                    break;
                default:
                    mexErrMsgTxt("Unsupported class for variable data.");
                    return;
                }
        }   
    }//loop over all the fields

}

harp_product *harp_matlab_set_product(const mxArray *mx_struct)
{
    harp_product *product;
    int num_variables;
    int index;

    if (!mxIsStruct(mx_struct))
    {
        mexErrMsgTxt("Not a struct.");
    }
    num_variables = mxGetNumberOfFields(mx_struct);
    // here the number is bigger than the variables number, due to the meta information
    // mexPrintf("!!!!!! this should be: %d \n", num_variables);

    if (harp_product_new(&product)!= 0)
    {
        harp_matlab_harp_error();
    }

    for (index = 0; index < num_variables; index++)
    {
        const char *variable_name;
        
        variable_name = mxGetFieldNameByNumber(mx_struct, index);
     
        /*------set meta info-------*/
        if(strncmp(variable_name,"source",6)==0){
            mxArray * meta  = mxGetField(mx_struct, index, variable_name);
            char * metastring = mxArrayToString(meta);
            (*product).source_product = metastring;
            mxFree(metastring);
        }
        else if(strncmp(variable_name, "history",7)==0){
            mxArray * meta  = mxGetField(mx_struct, index, variable_name);
            char * metastring = mxArrayToString(meta);
            (*product).history = metastring;
            mxFree(metastring);
        }
        else{
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
                /* to check  **/
                mx_dim_variable = NULL;
                dim_variable_name = create_num_dims_variable(variable_name);
                // dim_variable_name = variable_name;
                // dim_variable_index = mxGetFieldNumber(mx_struct, dim_variable_name);
                dim_variable_index = mxGetFieldNumber(mx_struct, variable_name);
                // int number = mxGetNumberOfFields(mx_struct);
                // mexPrintf("number of the fields is: %d \n", number);
                // mexPrintf(" dim_variable_name is: %s \n", dim_variable_name);
                // mexPrintf(" dim_variable_index is: %d \n", dim_variable_index);
                mxFree(dim_variable_name);

                if (dim_variable_index >= 0)
                {   
                    // mx_dim_variable = mxGetField(mx_struct,0, dim_variable_name);

                    mx_dim_variable = mxGetFieldByNumber(mx_struct,0, dim_variable_index);
                    // int elements = mxGetNumberOfElements(mx_dim_variable);
                    dim1 = mxGetNumberOfDimensions(mx_dim_variable);// matlab dim1 is always 2
                    dim2 = mxGetDimensions(mx_dim_variable);
                    // mexPrintf("dimension of the variable from matlab is: %d \n", dim1);
                }

                mx_variable = mxGetFieldByNumber(mx_struct, 0, index);
        
                harp_matlab_add_matlab_product_variable(&product, variable_name, mx_variable, dim1);
            
            }
       }

        mexPrintf("~~~~~ %d ~~~~~:\n", index);
    }

    return product;
}