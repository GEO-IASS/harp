function varargout = beatl2_append(varargin)
% BEATL2_APPEND  Append two BEAT-II records.
%
%   RECORD = BEATL2_APPEND(RECORD_1, RECORD_2) will append record_2 to record_1
%   using the main (i.e. first) dimension.
%
%   Both records need to contain the same set of fields.
%
%   More information about BEAT-II records can be found in the BEAT-II Data
%   Description documentation.
%

% Call BEATL2_MATLAB.MEX to do the actual work.
[varargout{1:max(1,nargout)}] = beatl2_matlab('APPEND',varargin{:});
