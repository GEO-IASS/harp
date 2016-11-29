function varargout = beatl2_export(varargin)
% BEATL2_EXPORT  Export a BEAT-II record to a file.
%
%   BEATL2_EXPORT(FORMAT, FILEPATH, RECORD) will export the given record
%   to the file that is specified by filepath. The export formats that
%   are supported are:
%     - ASCII
%     - BINARY
%     - HDF4
%     - HDF5
%   You should provide the format as a string to BEATL2_EXPORT. If the
%   export file already exists it will be overwritten.
%
%   See also BEATL2_IMPORT
%

% Call BEATL2_MATLAB.MEX to do the actual work.
beatl2_matlab('EXPORT',varargin{:});
