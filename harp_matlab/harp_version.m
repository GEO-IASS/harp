function varargout = beatl2_version(varargin)
% BEATL2_VERSION  Get version number of BEAT.
%
%   VERSION = BEATL2_VERSION returns the version number of BEAT.
%

% Call BEATL2_MATLAB.MEX to do the actual work.
[varargout{1:max(1,nargout)}] = beatl2_matlab('VERSION',varargin{:});
