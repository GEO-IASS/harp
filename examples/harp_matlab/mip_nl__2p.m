function mip_nl__2p(directory)
% MIP_NL__2P Show MIPAS level 2 data with HARP.
%
%    MIP_NL__2P(directory) shows O3 profiles for one or more
%    MIPAS level 2 products.
%

% find all files in the specified directory.
files = dir(strcat(directory,'/MIP_NL__2P*.N1'));

num_files = length(files);

if num_files == 0
  disp('WARNING: no files found!');
end

for i=1:num_files
  files(i).name = strcat(directory,'/',files(i).name);
end
filenames = strvcat(files.name);
data = harp_ingest(filenames);

scatter3(data.longitude, data.latitude, data.altitude, 4, log10(data.o3_vmr));
xlabel('longitude [ deg ]');
ylabel('latitude [ deg ]');
zlabel('tangent height [ km ]');
title('MIPAS Level-2');
axis([-180 180 -90 90 0 80]);
caxis([-4 5]);
colorbar('horz');
val = data.o3_vmr;
