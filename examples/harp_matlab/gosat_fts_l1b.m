function gosat_fts_l1b(filename)
% GOSAT_FTS_L1B Show GOSAT FTS Level-1 Radiance spectra with BEAT-II
%
%    GOSAT_FTS_L1B(filename) shows transmission data from a 
%      GOSAT FTS L1b product file.
%

% available bands are: 'band1p', 'band1s', 'band2p', 'band2s',
%                      'band3p', 'band3s', and 'band4'.
band = 'band4';

% add a filter option that specifies from which band we want the radiance data
filter = sprintf('data=%s', band);

% we can optionally add a wavenumber range limit on our ingestion to eliminate
% the 'uninteresting' edges of the band.
% Suggested ranges for each band are:
% band1p/s: 12900-13200
%filter = sprintf('%s,wavenumber_min=12900,wavenumber_max=13200', filter);
% band2p/s: 5800-6400
%filter = sprintf('%s,wavenumber_min=5800,wavenumber_max=6400', filter);
% band3p/s: 4800-5200
%filter = sprintf('%s,wavenumber_min=4800,wavenumber_max=5200', filter);
% band4: 650-*
filter = sprintf('%s,wavenumber_min=650', filter);

data = beatl2_ingest(filename,filter);

for i=1:length(data.time)
  % plot the spectrum and some annotation information
  plot(data.wavenumber(i,:), data.spectral_radiance(i,:));
  xlabel('wavenumber [ nm ]');
  ylabel(sprintf('radiance [ %s ]', data.spectral_radiance_unit));
  title(sprintf('GOSAT FTS L1b %s, time: %s', band, coda_time_to_string(data.time(i))));

  pause(0.01);

end
