import matlab.io.hdf4.*
sdID = sd.start('mydata.hdf4','create');
A = [1 2 3 4 5 ; 6 7 8 9 10 ; 11 12 13 14 15];
ds_name = 'A';
ds_type = 'double';
ds_dims = size(A);
sdsID = sd.create(sdID,ds_name,ds_type,ds_dims);

start = [0 0];
sd.writeData(sdsID,start,A);

sd.setAttr(sdID,'creation_date',datestr(now));
attr_name = 'cordsys';
attr_value = 'polar';
sd.setAttr(sdsID,attr_name,attr_value);
sd.endAccess(sdsID);
sd.close(sdID);
