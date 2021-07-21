% This file shows some example usage of the Matlab OCT scripts
% To use exectute this test file, an OCT dataset name 'testdata.oct' 
% containing a single BScan has to put into this directory

handle = OCTFileOpen('testdata.oct');

%%%%% read dataset properties %%%%%%

disp( OCTFileGetProperty(handle, 'AcquisitionMode') );
disp( OCTFileGetProperty(handle, 'RefractiveIndex') );
disp( OCTFileGetProperty(handle, 'Comment') );
disp( OCTFileGetProperty(handle, 'Study') );
disp( OCTFileGetProperty(handle, 'ExperimentNumber') );

%%%%% reading intensity %%%%%%

Intensity = OCTFileGetIntensity(handle);
figure();clf;
% a = [1 5 10; 15 20 25; 30 35 40];
imagesc(Intensity(:,:,1));

%%%%% reading video image %%%%%%

VideoImage = OCTFileGetColoredData(handle,'VideoImage');
figure(2);clf;
imagesc(VideoImage);

%%%%% reading chirp vector %%%%%%

Chirp = OCTFileGetChirp(handle);
figure(3);clf;
plot(Chirp);

%%%%% reading spectral raw data %%%%%%

NrRawData = OCTFileGetNrRawData(handle);

[RawData, Spectrum] = OCTFileGetRawData(handle, 0);
figure(4);clf;
plot(Spectrum);
figure(5);clf;
imagesc(RawData);

%%%%% close OCT file (deletes temporary files) %%%%%%

OCTFileClose(handle);
