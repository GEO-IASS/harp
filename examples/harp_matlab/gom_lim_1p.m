function gom_lim_1p(filename)
% GOM_LIM_1P Show GOMOS Level-1 Background spectra with HARP
%
%    GOM_LIM_1P(filename) shows the four types of background spectra in
%    a GOMOS Limb product for the first measurement in the GOM_LIM_1P file.
%

upper_no_cor = beatl2_ingest(filename,'spectral_background_variant=upper uncorrected');
upper_cor = beatl2_ingest(filename,'spectral_background_variant=upper corrected');
lower_no_cor = beatl2_ingest(filename,'spectral_background_variant=lower uncorrected');
lower_cor = beatl2_ingest(filename,'spectral_background_variant=lower corrected');

range(1) = min(min(upper_no_cor.wavelength_per_profile));
range(2) = max(max(upper_no_cor.wavelength_per_profile));
range(3) = 1000;
range(4) = 100000;

figure;
semilogy(upper_no_cor.wavelength_per_profile,upper_no_cor.spectral_background(1,:));
axis(range);
xlabel('wavelength [nm]');
ylabel('[e]');
title('Uncalibrated upper background band spectra');

figure;
semilogy(upper_cor.wavelength_per_profile,upper_cor.spectral_background(1,:));
axis(range);
xlabel('wavelength [nm]');
ylabel('[e]');
title('Calibrated upper background band spectra');

figure;
semilogy(lower_no_cor.wavelength_per_profile,lower_no_cor.spectral_background(1,:));
axis(range);
xlabel('wavelength [nm]');
ylabel('[e]');
title('Uncalibrated lower background band spectra');

figure;
semilogy(lower_cor.wavelength_per_profile,lower_cor.spectral_background(1,:));
axis(range);
xlabel('wavelength [nm]');
ylabel('[e]');
title('Calibrated lower background band spectra');
