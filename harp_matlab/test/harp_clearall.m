function varargout = beatl2_clearall(varargin)
% BEATL2_CLEARALL Free all allocated resources that are claimed by
%   BEATL2-MATLAB. 
%
%   BEATL2_CLEARALL will clean up any BEAT-II resources. At the first
%   call to a BEATL2-MATLAB function the BEAT-II C Library will be
%   initialized which will create a full representation of the BEAT-I
%   data dictionary in memory (which can be a few megabytes in size). A
%   call to BEATL2_CLEARALL will clean up any BEAT-II resources and thus
%   also remove the data dictionary. After a clean up, the first call to
%   a BEATL2-MATLAB function will initialize the BEAT-II C Library again.
%
%   This function may be (slightly) useful on systems with little
%   memory. You could ingest a product file and then unload the data
%   dictionary to free an extra megabyte of memory before starting
%   data-processing. Other than that, this function is of little
%   practical use.
%

% Call BEATL2_MATLAB.MEX to do the actual work.
beatl2_matlab('CLEARALL',varargin{:});
