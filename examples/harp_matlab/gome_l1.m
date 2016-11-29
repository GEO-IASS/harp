function gome_l1(filename)
% GOME_L1 Show GOME Level-1 data with BEAT-II.
%
%    GOME_L1(filename) shows the spectral readouts for a single
%    'GOME_L1' product file with BEAT-II.
%

data = beatl2_ingest(filename);
time = data.time;
band = data.spectral_readout;
band_wav = data.wavelength;

num_measurements = length(time);

for i=1:num_measurements
  semilogy(band_wav(i,:),band(i,:),'r');
  title(sprintf('GOME Level 1 %s', beat_time_to_string(time(i))));
  xlabel('wavelength [nm]');
  ylabel('[BU]');
  axis manual;
  axis([200 850 100 65535]);
  pause(0.01);
end
