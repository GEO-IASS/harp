function varargout = beatl2_find_colocated_data(varargin)
% BEATL2_FIND_COLOCATED_DATA  Find co-located data for two BEAT-II records
%   matching time, geolocation, and/or altitude.
%
%   [RECORD_1, RECORD_2] = BEATL2_FIND_COLOCATED_DATA(RECORD_1, RECORD_2,
%   TIME_DISTANCE, RADIAL_DISTANCE, ALTITUDE_DISTANCE) will find the 
%   measurements in record_1 and record_2 that are close together
%   (i.e. co-located). In order for the co-location to work both records
%   need to have at least two similar fields to match on (time, latitude,
%   longitude, and altitude). Each of these fields that are in a record need
%   to be one dimensional.
%   If both records have a valid time field then the time_distance parameter
%   (distance is seconds) is used to match the time values.
%   If both records have valid geolocation fields (latitude and longitude)
%   then the radial_distance parameter (which represents an earth surface
%   distance in km) is used to match the geolocation values.
%   If both records have a valid altitude field then the altitude_distance
%   parameter (in km) is used to match the altitude values.
%   The final result is the intersection of the results for each of the fields.
%   This means that the records that are returned only contain the measurements
%   that lie within the given distances.
%
%   If a distance parameter is negative then no comparison will be performed
%   for the associated property. If the function is unable to perform any
%   comparisons (because all distance parameters are negative) the function
%   will return an error.
%
%   More information about BEAT-II records can be found in the BEAT-II Data
%   Description documentation.
%

% Call BEATL2_MATLAB.MEX to do the actual work.
[varargout{1:max(1,nargout)}] = beatl2_matlab('FIND_COLOCATED_DATA',varargin{:});
