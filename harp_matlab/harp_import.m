function varargout = beatl2_import(varargin)
% BEATL2_IMPORT  Import a BEAT-II record from a file.
%
%   RECORD = BEATL2_IMPORT(FORMAT, FILEPATH) will import the record
%   stored in the file that is specified by filepath. You should specify
%   the format of the file with the format parameter. Possible values
%   for format are:
%     - ASCII
%     - BINARY
%     - HDF4
%     - HDF5
%   You should provide the format as a string to BEATL2_IMPORT.
%
%   See also BEATL2_EXPORT
%

% Call BEATL2_MATLAB.MEX to do the actual work.
[varargout{1:max(1,nargout)}] = beatl2_matlab('IMPORT',varargin{:});
