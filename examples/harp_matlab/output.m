function output(filename)
data = harp_ingest(filename);
%variable =  data.datetime_start;
%display(data.datetime_start);
%display(variable.value);
%display(data.scan_direction);
%harp_export(filename, result, product) 
%harp_export('result.nc','netcdf', data.value);
harp_export('result.nc','netcdf', data);
%harp_export('result.hdf4','hdf4', data);



