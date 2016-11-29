function varargout = beatl2_slice(varargin)
% BEATL2_SLICE  Shrink a BEAT-II record to a subrange of its data.
%
%   RECORD = BEATL2_SLICE(RECORD, RANGE) will shrink the record in the
%   main dimension.
%
%   RECORD = BEATL2_SLICE(RECORD, DIMENSION, RANGE) will shrink the record in
%   the given dimension (which should be a sub-dimension of the main
%   dimension).
%
%   The range should be a range of integers representing the indices (with
%   regard to the main dimension) of the elements that should be kept.
%
%   More information about BEAT-II records can be found in the BEAT-II Data
%   Description documentation.
%

% Call BEATL2_MATLAB.MEX to do the actual work.
[varargout{1:max(1,nargout)}] = beatl2_matlab('SLICE',varargin{:});
