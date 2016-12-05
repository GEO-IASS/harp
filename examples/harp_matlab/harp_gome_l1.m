function harp_gome_l1(filename)
% SCI_OL__2P Show SCIAMACHY Level-2 Offline data with BEAT-II.
%
%    SCI_OL__2P(directory) shows O3 vcd data from one or more
%    'SCI_OL__2P' product files with BEAT-II.
%

netherlands = [  -5   15  50  60];
europe      = [ -20   40  40  70];
world       = [-180  180 -90  90];
custom      = [  28   67  69  79];

% frame = world;
% 
% % find all files in the specified directory.
% % files = dir(strcat(directory,'/SCI_OL__2P*.N1'));
% 
% num_files = length(files);
% 
% if num_files == 0 
%   disp('WARNING: no files found!');
% end
% 
% for i=1:num_files
%   files(i).name = strcat(directory,'/',files(i).name);
% end
% filenames = strvcat(files.name);
% filter = sprintf('data=nadir_uv_0,convert_to_DU,latitude_min=%d,latitude_max=%d,longitude_min=%d,longitude_max=%d,scan_direction=forward mixed', frame(3), frame(4), frame(1), frame(2));
% data = beatl2_ingest(filenames,filter);
% 
% figure;
% plot_geo_pixel(data, 'o3_column');
% title('Sciamachy Level-2 Ozone');
% axis(frame);
% caxis([150 500]);

data = harp_ingest(filename);
display(data)



