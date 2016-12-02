/*
 * Copyright (C) 2002-2016 S[&]T, The Netherlands.
 *
 * This file is part of HARP.
 *
 */

#include "harp-matlab.h"

#include <string.h>

#include "coda.h"
#include "harp.h"

#define MAX_FUNCNAME_LENGTH     50

static int harp_matlab_initialised = 0;

static void harp_matlab_cleanup();

// static void harp_matlab_append(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);
// static void harp_matlab_clearall(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);
static void harp_matlab_export(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);
// static void harp_matlab_find_colocated_data(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);
static void harp_matlab_import(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);
static void harp_matlab_ingest(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);
// static void harp_matlab_slice(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);
static void harp_matlab_version(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);

void harp_matlab_harp_error(void)
{
    mexPrintf("ERROR : %s\n", harp_errno_to_string(harp_errno));
    mexErrMsgTxt("harp Error");
}

static void harp_matlab_cleanup(void)
{
    /* clean up harp */
    if (harp_matlab_initialised)
    {
        harp_done();
        harp_matlab_initialised = 0;
    }
}

static void harp_matlab_set_definition_path(void)
{
    if (getenv("CODA_DEFINITION") == NULL)
    {
        mxArray *mxpath;
        mxArray *arg;
        char *path;
        int path_length;

        arg = mxCreateString("harp_version");
        if (mexCallMATLAB(1, &mxpath, 1, &arg, "which") != 0)
        {
            mexErrMsgTxt("Could not retrieve module path");
        }
        mxDestroyArray(arg);

        path_length = mxGetN(mxpath) * mxGetM(mxpath) + 1;
        path = mxCalloc(path_length + 1, 1);
        if (mxGetString(mxpath, path, path_length + 1) != 0)
        {
            mexErrMsgTxt("Error copying string");
        }
        /* remove 'harp_version.m' from path */
        if (path_length > 16)
        {
            path[path_length - 16 - 1] = '\0';
        }
        mxDestroyArray(mxpath);
#ifdef CODA_DEFINITION_MATLAB
        coda_set_definition_path_conditional("harp_version.m", path, CODA_DEFINITION_MATLAB);
#else
        coda_set_definition_path_conditional("harp_version.m", path, "../../../share/" PACKAGE "/definitions");
#endif
        mxFree(path);
    }
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    char funcname[MAX_FUNCNAME_LENGTH + 1];

    harp_set_error(HARP_SUCCESS, NULL);

    if (!harp_matlab_initialised)
    {
        harp_matlab_set_definition_path();

        if (harp_init() != 0)
        {
            harp_matlab_harp_error();
        }
        harp_matlab_initialised = 1;
        mexAtExit(&harp_matlab_cleanup);
    }

    /* check parameters */
    if (!(nrhs >= 1 && mxIsChar(prhs[0]) && mxGetM(prhs[0]) == 1 && mxGetN(prhs[0]) <= MAX_FUNCNAME_LENGTH))
    {
        mexErrMsgTxt("Incorrect invocation of harp-MATLAB gateway function.");
    }

    if (mxGetString(prhs[0], funcname, MAX_FUNCNAME_LENGTH + 1) != 0)
    {
        mexErrMsgTxt("Error in harp-MATLAB gateway function: Could not copy string.");
    }

  
    if (strcmp(funcname, "IMPORT") == 0)
    {
        harp_matlab_import(nlhs, plhs, nrhs - 1, &(prhs[1]));
    }
    else if (strcmp(funcname, "INGEST") == 0)
    {
        harp_matlab_ingest(nlhs, plhs, nrhs - 1, &(prhs[1]));
    }
    else if (strcmp(funcname, "EXPORT") == 0)
    {
        harp_matlab_export(nlhs, plhs, nrhs - 1, &(prhs[1]));
    }
    else if (strcmp(funcname, "VERSION") == 0)
    {
        harp_matlab_version(nlhs, plhs, nrhs - 1, &(prhs[1]));
    }
    else
    {
        mexErrMsgTxt("Error in harp-MATLAB gateway function: Unknown function name.");
    }
}

// static void harp_matlab_append(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
// {
//     harp_product *product_1;
//     harp_product *product_2;

//     /* check parameters */
//     if (nlhs > 1)
//     {
//         mexErrMsgTxt("Too many output arguments.");
//     }
//     if (nrhs != 2)
//     {
//         mexErrMsgTxt("Function takes exactly two arguments.");
//     }

//     product_1 = harp_matlab_set_product(prhs[0]);
//     product_2 = harp_matlab_set_product(prhs[1]);

//     if (harp_product_add_variable(product_1, product_2) != 0)
//     {
//         harp_product_delete(product_2);
//         harp_product_delete(product_1);
//         harp_matlab_harp_error();
//     }
//     harp_product_delete(product_2);

//     plhs[0] = harp_matlab_get_product(product_1);
//     harp_product_delete(product_1);
// }

// static void harp_matlab_clearall(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
// {
//     (void)plhs; /* prevents 'unused parameter' warning */
//     (void)prhs; /* prevents 'unused parameter' warning */

//     /* check parameters */
//     if (nlhs > 0)
//     {
//         mexErrMsgTxt("Too many output arguments.");
//     }
//     if (nrhs != 0)
//     {
//         mexErrMsgTxt("Function takes no arguments.");
//     }

//     harp_matlab_cleanup();
// }

static void harp_matlab_export(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    harp_product *product;
    char *format;
    char *filename;
    int buflen;

    (void)plhs; /* prevents 'unused parameter' warning */

    /* check parameters */
    if (nlhs > 0)
    {
        mexErrMsgTxt("Too many output arguments.");
    }
    if (nrhs != 3)
    {
        mexErrMsgTxt("Function takes exactly three arguments.");
    }
    if (!mxIsChar(prhs[0]))
    {
        mexErrMsgTxt("First argument should be a string.");
    }
    if (mxGetM(prhs[0]) != 1)
    {
        mexErrMsgTxt("First argument should be a row vector.");
    }
    if (!mxIsChar(prhs[1]))
    {
        mexErrMsgTxt("Second argument should be a string.");
    }
    if (mxGetM(prhs[1]) != 1)
    {
        mexErrMsgTxt("Second argument should be a row vector.");
    }

    buflen = mxGetN(prhs[0]) + 1;
    format = (char *)mxCalloc(buflen, sizeof(char));
    if (mxGetString(prhs[0], format, buflen) != 0)
    {
        mexErrMsgTxt("Unable to copy export format string.");
    }

    buflen = mxGetN(prhs[1]) + 1;
    filename = (char *)mxCalloc(buflen, sizeof(char));
    if (mxGetString(prhs[1], filename, buflen) != 0)
    {
        mexErrMsgTxt("Unable to copy filename string.");
    }

    product = harp_matlab_set_product(prhs[2]);

    if (harp_export(format, filename, product) != 0)
    {
        harp_product_delete(product);
        harp_matlab_harp_error();
    }
    harp_product_delete(product);

    mxFree(format);
    mxFree(filename);
}

// static void harp_matlab_find_colocated_data(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
// {
//     harp_product *product_1;
//     harp_product *product_2;

//     /* check parameters */
//     if (nlhs != 2)
//     {
//         mexErrMsgTxt("Functions needs exactly two output paramters.");
//     }
//     if (nrhs != 5)
//     {
//         mexErrMsgTxt("Function takes exactly five arguments.");
//     }

//     if (!mxIsDouble(prhs[2]))
//     {
//         mexErrMsgTxt("Third argument should be a double.");
//     }
//     if (!mxIsDouble(prhs[3]))
//     {
//         mexErrMsgTxt("Fourth argument should be a double.");
//     }
//     if (!mxIsDouble(prhs[4]))
//     {
//         mexErrMsgTxt("Fifth argument should be a double.");
//     }
//     product_1 = harp_matlab_set_product(prhs[0]);
//     product_2 = harp_matlab_set_product(prhs[1]);

//     if (harp_find_colocated_data(product_1, product_2, mxGetScalar(prhs[2]), mxGetScalar(prhs[3]), mxGetScalar(prhs[4]))
//         != 0)
//     {
//         harp_product_delete(product_1);
//         harp_product_delete(product_2);
//         harp_matlab_harp_error();
//     }

//     plhs[0] = harp_matlab_get_product(product_1);
//     plhs[1] = harp_matlab_get_product(product_2);
//     harp_product_delete(product_1);
//     harp_product_delete(product_2);
// }

static void harp_matlab_import(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    harp_product *product;
    char *format;
    char *filename;
    int buflen;

    (void)plhs; /* prevents 'unused parameter' warning */

    /* check parameters */
    if (nlhs > 1)
    {
        mexErrMsgTxt("Too many output arguments.");
    }
    if (nrhs != 2)
    {
        mexErrMsgTxt("Function takes exactly two arguments.");
    }
    if (!mxIsChar(prhs[0]))
    {
        mexErrMsgTxt("First argument should be a string.");
    }
    if (mxGetM(prhs[0]) != 1)
    {
        mexErrMsgTxt("First argument should be a row vector.");
    }
    if (!mxIsChar(prhs[1]))
    {
        mexErrMsgTxt("Second argument should be a string.");
    }
    if (mxGetM(prhs[1]) != 1)
    {
        mexErrMsgTxt("Second argument should be a row vector.");
    }

    buflen = mxGetN(prhs[0]) + 1;
    format = (char *)mxCalloc(buflen, sizeof(char));
    if (mxGetString(prhs[0], format, buflen) != 0)
    {
        mexErrMsgTxt("Unable to copy import format string.");
    }

    buflen = mxGetN(prhs[1]) + 1;
    filename = (char *)mxCalloc(buflen, sizeof(char));
    if (mxGetString(prhs[1], filename, buflen) != 0)
    {
        mexErrMsgTxt("Unable to copy filename string.");
    }

    if (harp_import(filename, &product) != 0)
    {
        harp_matlab_harp_error();
    }

    plhs[0] = harp_matlab_get_product(product);
    harp_product_delete(product);

    mxFree(format);
    mxFree(filename);
}

static void harp_matlab_ingest(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    harp_product *product;
    char *filename;
    // int num_files;
    char *script;
    char *option;
    int buflen;
    // int i;

    /* check parameters */
    if (nlhs > 1)
    {
        mexErrMsgTxt("Too many output arguments.");
    }
    if (nrhs != 1 && nrhs != 2)
    {
        mexErrMsgTxt("Function takes either one or two arguments.");
    }

    filename = NULL;
    // num_files = 0;

    if (!mxIsChar(prhs[0]))
    {
        mexErrMsgTxt("First argument should be a string.");
    }   
        // mxChar *str;

        // if (mxGetNumberOfDimensions(prhs[0]) > 2)
        // {
        //     mexErrMsgTxt("First argument should not be a char array with more then 2 dimensions.");
        // }
        buflen = mxGetN(prhs[0]) + 1;
        filename = (char *)mxCalloc(buflen, sizeof(char));
        // num_files = mxGetM(prhs[0]);
        // if (num_files == 0 || buflen == 1)
        // {
        //     mexErrMsgTxt("First argument contains an empty filelist.");
        // }
        // filename = (char **)mxCalloc(num_files, sizeof(char *));
        // str = mxGetData(prhs[0]);
        // for (i = 0; i < num_files; i++)
        // {
        //     int j;

        //     filenames[i] = (char *)mxCalloc(buflen, sizeof(char));
        //     for (j = 0; j < buflen - 1; j++)
        //     {
        //         filenames[i][j] = (char)str[j * num_files + i];
        //     }
        //     j = buflen;
        //     while (j > 0 && filenames[i][j - 1] == ' ')
        //     {
        //         j--;
        //     }
        //     filenames[i][j] = '\0';
        // }
    // }
    

    script = NULL;
    option = NULL;
    // if (nrhs == 2)
    // {
    if (!mxIsChar(prhs[1]))
    {
            mexErrMsgTxt("Second argument should be a string.");
   }
        // if (mxGetM(prhs[1]) != 1)
        // {
        //     mexErrMsgTxt("Second argument should be a row vector.");
        // }

        buflen = mxGetN(prhs[1]) + 1;
        script = (char *)mxCalloc(buflen, sizeof(char));
        // option = (char *)mxCalloc(buflen, sizeof(char));
        if (mxGetString(prhs[1], script, buflen) != 0)
        {
            mexErrMsgTxt("Unable to copy script string.");
        }
    // }
        if (!mxIsChar(prhs[2]))
        {
            mexErrMsgTxt("Third argument should be a string.");
        }
        buflen = mxGetN(prhs[2]) + 1;
        option = (char *)mxCalloc(buflen, sizeof(char));
        if (mxGetString(prhs[1], script, buflen) != 0)
        {
            mexErrMsgTxt("Unable to copy option string.");
        }


    if (harp_ingest(filename, script, option, &product) != 0)
    {
        harp_matlab_harp_error();
    }

    // for (i = 0; i < num_files; i++)
    // {
    //     mxFree(filenames[i]);
    // }
    mxFree(filename);
    if ( script!= NULL)
    {
        mxFree(script);
    }
    if ( option!= NULL)
    {
        mxFree(option);
    }


    plhs[0] = harp_matlab_get_product(product);
    harp_product_delete(product);

    if (plhs[0] == NULL)
    {
        harp_matlab_harp_error();
    }
}

// static void harp_matlab_slice(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
// {
//     harp_product *product;
//     char *dimension;
//     long num_dim_elements;
//     long *dim_element_id;
//     double *double_data;
//     long i;

//     /* check parameters */
//     if (nlhs > 1)
//     {
//         mexErrMsgTxt("Too many output arguments.");
//     }
//     if (nrhs != 2 && nrhs != 3)
//     {
//         mexErrMsgTxt("Function takes either two or three arguments.");
//     }

//     dimension = NULL;
//     if (nrhs == 3)
//     {
//         int buflen;

//         if (!mxIsChar(prhs[1]))
//         {
//             mexErrMsgTxt("Second argument should be a string.");
//         }
//         if (mxGetM(prhs[1]) != 1)
//         {
//             mexErrMsgTxt("Second argument should be a row vector.");
//         }
//         buflen = mxGetN(prhs[1]) + 1;
//         dimension = (char *)mxCalloc(buflen, sizeof(char));
//         if (mxGetString(prhs[1], dimension, buflen) != 0)
//         {
//             mexErrMsgTxt("Unable to copy export dimension string.");
//         }
//     }

//     if (!mxIsDouble(prhs[nrhs - 1]))
//     {
//         if (nrhs == 3)
//         {
//             mexErrMsgTxt("Third argument should be a numerical array with doubles.");
//         }
//         else
//         {
//             mexErrMsgTxt("Second argument should be a numerical array with doubles.");
//         }
//     }
//     num_dim_elements = mxGetNumberOfElements(prhs[nrhs - 1]);
//     dim_element_id = (long *)mxCalloc(num_dim_elements, sizeof(long));
//     double_data = (double *)mxGetData(prhs[nrhs - 1]);
//     for (i = 0; i < num_dim_elements; i++)
//     {
//         dim_element_id[i] = ((long)double_data[i]) - 1; /* go from 1 based to 0 based */
//     }
//     product = harp_matlab_set_product(prhs[0]);

//     if (harp_slice(product, dimension, num_dim_elements, dim_element_id) != 0)
//     {
//         harp_product_delete(product);
//         harp_matlab_harp_error();
//     }

//     plhs[0] = harp_matlab_get_product(product);
//     harp_product_delete(product);

//     mxFree(dim_element_id);
// }

static void harp_matlab_version(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    (void)prhs; /* prevents 'unused parameter' warning */

    /* check parameters */
    if (nlhs > 1)
    {
        mexErrMsgTxt("Too many output arguments.");
    }
    if (nrhs != 0)
    {
        mexErrMsgTxt("Function takes no arguments.");
    }

    plhs[0] = mxCreateString(VERSION);
}
