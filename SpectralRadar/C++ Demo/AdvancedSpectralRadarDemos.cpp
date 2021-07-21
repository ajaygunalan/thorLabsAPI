#include <iostream>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <sstream>
#include <SpectralRadar.h>


using namespace std;

void ReadOCTFile()
{
	// This example program shows how to read an oct-file with the SDK which has been acquired and saved with ThorImageOCT.
	// To make sure the correct parameters will be used to modify the loaded data, e.g. the files for the processing from the dataset and not the current ones for an acquisition, 
	// it is necessary to use the functions specified for an OCTFile as in this example.
	OCTFileHandle OCTFile = createOCTFile(FileFormat_OCITY);

	// Please select an .oct-file you want to load
	loadFile(OCTFile, "Example.oct");

	OCTDeviceHandle Dev = initDevice();
	ProbeHandle Probe = initProbeFromOCTFile(Dev, OCTFile);
	ProcessingHandle Proc = createProcessingForOCTFile(OCTFile);

	// with the functions handling the metadata the informations from the dataset can be loaded
	double RangeX = getFileMetadataFloat(OCTFile, FileMetadata_RangeX);
}

void WriteOCTFile()
{
	// This example program shows how to write data acquired with the SDK to an oct-file which can be viewed with ThorImageOCT.
	char message[1024];

	OCTDeviceHandle Dev = initDevice();
	ProbeHandle Probe = initProbe(Dev, "Probe");
	ProcessingHandle Proc = createProcessingForDevice(Dev);

	RawDataHandle Raw = createRawData();
	DataHandle BScan = createData();
	ColoredDataHandle VideoImg = createColoredData();

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	// Creating the pattern with no range in the second direction put all B-scans to the same position.
	ScanPatternHandle Pattern = createBScanPattern(Probe, 2, 1024);

	startMeasurement(Dev, Pattern, Acquisition_AsyncFinite);
	
	getRawData(Dev, Raw);
	getCameraImage(Dev, VideoImg);

	setProcessedDataOutput(Proc, BScan);

	executeProcessing(Proc, Raw);

	stopMeasurement(Dev);

	// Creating the file in the correct data format for ThorImageOCT
	OCTFileHandle OCTFile = createOCTFile(FileFormat_OCITY);

	// Adding the processed data to the file
	// To see what data types are used in standard .oct-files please see variables "DataObjectName_" in "SpectralRadar.h" 
	addFileRealData(OCTFile, BScan, DataObjectName_OCTData);
	
	// Save video camera image
	addFileColoredData(OCTFile, VideoImg, DataObjectName_VideoImage);

	// A suitable mode need to be selected to view the data in ThorImageOCT. 
	// All acquisition modes from ThorImageOCT are listed in "SpectralRadar.h" starting with "AcquisitionMode_".
	// Please note that the available acquisitions modes may depend on your hardware.
	setFileMetadataString(OCTFile, FileMetadata_AcquisitionMode, AcquisitionMode_2D);
	// The data from the device, probe, processing and scan pattern will be saved as well
	saveFileMetadata(OCTFile, Dev, Proc, Probe, Pattern);
	saveFile(OCTFile, "AcquiredOCTFileWithSDK.oct");

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	clearOCTFile(OCTFile);
	clearScanPattern(Pattern);

	clearColoredData(VideoImg);
	clearData(BScan);
	clearRawData(Raw);

	clearProcessing(Proc);
	closeProbe(Probe);
	closeDevice(Dev);

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}
}

void ProcessingChain()
{
	// The processing chain consists out of several steps. With the SDK it is possible to get the processed data in between and not only at the end. 
	// This may be useful if you want to write your own processing routine and do not want ti reimplement all of it.
	char message[1024];

	OCTDeviceHandle Dev = initDevice();
	ProbeHandle Probe = initProbe(Dev, "Probe");
	ProcessingHandle Proc = createProcessingForDevice(Dev);

	RawDataHandle Raw = createRawData();
	DataHandle SpectrumOffsetsRemoved = createData();
	DataHandle BScan = createData();

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	// Creating the pattern with no range in the second direction put all B-scans to the same position.
	ScanPatternHandle Pattern = createBScanPattern(Probe, 2, 1024);

	startMeasurement(Dev, Pattern, Acquisition_AsyncFinite);

	getRawData(Dev, Raw);

	// The output from the processing routine can be set to different steps from the processing routine. 
	// Here the offset corrected spectrum is chosen.
	// Several others are available, e.g. #setSpectrumOutput, #setDCCorrectedSpectrumOutput and #setApodizedSpectrumOutput.
	// Please see the chapter Processing in the documentation for more information.
	
	// Spectrum after the offsets are removed only
	setOffsetCorrectedSpectrumOutput(Proc, SpectrumOffsetsRemoved);
	// Processed data with the complete processing chain applied.
	setProcessedDataOutput(Proc, BScan);

	executeProcessing(Proc, Raw);

	stopMeasurement(Dev);

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	clearScanPattern(Pattern);

	clearData(BScan);
	clearData(SpectrumOffsetsRemoved);
	clearRawData(Raw);

	clearProcessing(Proc);
	closeProbe(Probe);
	closeDevice(Dev);

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}
}

void AdvancedModificationOfScanPattern()
{
	// With our SDK it is possible to create a scan pattern consisting out of several B-scans acquired after each other directly. All B-scans need to have the same number of A-scans. 
	// Therefore it is possible to create a pattern of rotating B-scans.
	// One way to create such a pattern is to create a volume pattern first and modify it with the functions #shiftScanPatternEx and #rotateScanPatternEx.
	// This two functions shifts/rotate a single B-scan out of the volume pattern. This example shows how to use the function #rotateScanPatternEx, the use of #shiftScanPatternEx is analogous. 
	char message[1024];

	OCTDeviceHandle Dev = initDevice();
	ProbeHandle Probe = initProbe(Dev, "Probe");
	ProcessingHandle Proc = createProcessingForDevice(Dev);
	
	RawDataHandle Raw = createRawData();
	DataHandle BScan = createData();

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	int AScansPerBScan = 1024;
	double LengthOfBScan = 2;
	int NumberOfBScans = 16;

	// creating the pattern with no range in the second direction put all B-scans to the same position.
	ScanPatternHandle Pattern = createVolumePattern(Probe, LengthOfBScan, AScansPerBScan, 0.0, NumberOfBScans, ScanPattern_ApoEachBScan, ScanPattern_AcqOrderAll);

	// Rotating the 16 B-scans
	double StepSize = 360 / NumberOfBScans;
	StepSize *= (3.14159265 / 180); // the parameter for rotation is in radians and not degree
	// the first B-scan will not be rotated
	for (int CurrentBScan = 1; CurrentBScan < NumberOfBScans; ++CurrentBScan)
		rotateScanPatternEx(Pattern, CurrentBScan * StepSize, CurrentBScan);

	startMeasurement(Dev, Pattern, Acquisition_AsyncFinite);

	getRawData(Dev, Raw);

	setProcessedDataOutput(Proc, BScan);
	executeProcessing(Proc, Raw);

	stopMeasurement(Dev);

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	clearScanPattern(Pattern);

	clearData(BScan);
	clearRawData(Raw);

	clearProcessing(Proc);
	closeProbe(Probe);
	closeDevice(Dev);

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}
}

void FreeformScanPatterns()
{
	// With the freeform scan pattern functions it is possible to create 2D and 3D scan patterns of arbitrary form. 
	// The points used to create the scan pattern can be either only edge points of the pattern and by using interpolation methods the real scan points will be created.
	// The other possibility is that the user create all scan positions which will be used "as is".

	char message[1024];

	OCTDeviceHandle Dev = initDevice();

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	ProbeHandle Probe = initProbe(Dev, "Probe");
	ProcessingHandle Proc = createProcessingForDevice(Dev);

	// First two examples to define only the edge points of the scan pattern and let the real scan positions created inside the SDK will be shown.
	// Create points used for the scan pattern. These coordinates are in mm.
	vector<double> PosX = { 0, -1, 0, 1 };
	vector<double> PosY = { 1, 0, -1, 0 };
	vector<int> ScanIndices = { 0, 0, 0, 0 };

	int AScansInBScan = 512;
	ScanPatternHandle Pattern = createFreeformScanPattern2D(Probe, PosX.data(), PosY.data(), PosX.size(), AScansInBScan, Interpolation_Spline, true);

	// save scan points from the scan pattern to a file
	int ScanPatternSize = getScanPointsSize(Pattern);
	PosX.resize(ScanPatternSize);
	PosY.resize(ScanPatternSize);
	ScanIndices.resize(ScanPatternSize);
	getScanPoints(Pattern, PosX.data(), PosY.data());
	saveScanPointsToFile(PosX.data(), PosY.data(), ScanIndices.data(), ScanPatternSize, "ScanPatternPoints", ScanPoints_DataFormat_TXT);

	// To create a 3D-freeform scan pattern the "edge" points for both B-scans need to be defined and the corresponding scan indices. 
	PosX = { 0, -1, 0, 1, 0, -2, 0, 2 };
	PosY = { 1, 0, -1, 0, 2, 0, -2, 0 };
	ScanIndices = { 0, 0, 0, 0, 1, 1, 1, 1 };
	Pattern = createFreeformScanPattern3D(Probe, PosX.data(), PosY.data(), ScanIndices.data(), PosX.size(), AScansInBScan, Interpolation_Spline, true, ScanPattern_ApoEachBScan, ScanPattern_AcqOrderAll);


	// The following two examples using the function createFreefromScanPatternXDFromLUT create a scan pattern with defined scan points from the user directly.
	vector<double> ScanPosX(AScansInBScan);
	vector<double> ScanPosY(AScansInBScan);

	PosX = { 0, -1, 0, 1 };
	PosY = { 1, 0, -1, 0 };
	ScanIndices = { 0, 0, 0, 0 };

	// using an interpolation function to create te scan points
	interpolatePoints2D(PosX.data(), PosY.data(), PosX.size(), ScanPosX.data(), ScanPosY.data(), AScansInBScan, Interpolation_Spline, BoundaryCondition_Periodic);
	Pattern = createFreeformScanPattern2DFromLUT(Probe, ScanPosX.data(), ScanPosY.data(), ScanPosX.size(), true);

	int NumberOfBScans = 2;
	vector<double> ScanPosX_3D(AScansInBScan * NumberOfBScans);
	vector<double> ScanPosY_3D(AScansInBScan * NumberOfBScans);
	inflatePoints(ScanPosX.data(), ScanPosY.data(), ScanPosX.size(), ScanPosX_3D.data(), ScanPosY_3D.data(), NumberOfBScans, 1.0, Inflation_NormalDirection);
	Pattern = createFreeformScanPattern3DFromLUT(Probe, ScanPosX_3D.data(), ScanPosY_3D.data(), AScansInBScan, NumberOfBScans, true, ScanPattern_ApoEachBScan, ScanPattern_AcqOrderAll);

	// The points from the scan pattern can be read with the function #getScanPatternLUT. Fir this first the size of the pattern need to be known which is available with #getScanPatternLUTSize
	ScanPatternSize = getScanPatternLUTSize(Pattern);
	PosX.resize(ScanPatternSize);
	PosY.resize(ScanPatternSize);
	getScanPatternLUT(Pattern, PosX.data(), PosY.data());
	ScanIndices.resize(ScanPatternSize, 0);

	// The created scan points can be loaded/saved from/to a file with #loadScanPointsFromFile or #saveScanPointsToFile in different formats #ScanPointsDataFormat
	// Make sure to pass an arrays of appropiate size to #loadScanPointsFromFile which can be get with #getSizeOfScanPointsFromFile
	saveScanPointsToFile(PosX.data(), PosY.data(), ScanIndices.data(), ScanPatternSize, "C:\\tmp\\ScanPoints", ScanPoints_DataFormat_TXT);

	ScanPatternSize = getScanPointsSize(Pattern);
	PosX.resize(ScanPatternSize);
	PosY.resize(ScanPatternSize);
	getScanPoints(Pattern, PosX.data(), PosY.data());
	ScanIndices.resize(ScanPatternSize, 0);
	saveScanPointsToFile(PosX.data(), PosY.data(), ScanIndices.data(), ScanPatternSize, "C:\\tmp\\ScanPoints_mm", ScanPoints_DataFormat_TXT);


	if (getError(message, 512))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	RawDataHandle Raw = createRawData();
	DataHandle BScan = createData();


	startMeasurement(Dev, Pattern, Acquisition_AsyncFinite);

	if (getError(message, 512))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	getRawData(Dev, Raw);

	setProcessedDataOutput(Proc, BScan);
	executeProcessing(Proc, Raw);

	stopMeasurement(Dev);

	clearRawData(Raw);
	clearData(BScan);

	clearScanPattern(Pattern);
	closeProbe(Probe);
	clearProcessing(Proc);
	closeDevice(Dev);

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}
}

void RemovingApoFromScanPattern()
{
	// Sometimes it can be useful to get rid off the acquisition of additional apodization spectra used in the processing routine to speed up the acquisition process.
	// It is possible to perform the acquisition of those apodization spectra before the measurement is started and use those spectra in the processing chain. 
	char message[1024];

	OCTDeviceHandle Dev = initDevice();
	ProbeHandle Probe = initProbe(Dev, "Probe");
	ProcessingHandle Proc = createProcessingForDevice(Dev);

	RawDataHandle Raw = createRawData();
	DataHandle BScan = createData();

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	// Sets the number of apodization spectra to zero and therefore no apodization is performed in the scan later. 
	// Also the time for the scanner to get to the starting position of each scan is reduced. Only once the flyback time is needed instead of two. 
	setProbeParameterInt(Probe, Probe_ApodizationCycles, 0);

	int AScansPerBScan = 1024;
	double LengthOfBScan = 2;

	ScanPatternHandle Pattern = createBScanPattern(Probe, LengthOfBScan, AScansPerBScan);

	// the apodization spectra are acquired now
	measureApodizationSpectra(Dev, Probe, Proc);

	startMeasurement(Dev, Pattern, Acquisition_AsyncContinuous);
	while (!_kbhit())
	{
		getRawData(Dev, Raw);

		setProcessedDataOutput(Proc, BScan);
		executeProcessing(Proc, Raw);
	}
	stopMeasurement(Dev);

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	clearScanPattern(Pattern);

	clearData(BScan);
	clearRawData(Raw);

	clearProcessing(Proc);
	closeProbe(Probe);
	closeDevice(Dev);

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}
}

void DopplerOCT()
{
	// This demo program shows how to perform the additional processing used for Doppler OCT imaging.
	char message[1024];

	OCTDeviceHandle Dev = initDevice();
	ProbeHandle Probe = initProbe(Dev, "Probe");
	ProcessingHandle Proc = createProcessingForDevice(Dev);

	// create additional processing handling for Doppler OCT
	DopplerProcessingHandle DopplerProc = createDopplerProcessing();

	RawDataHandle Raw = createRawData();
	ComplexDataHandle ComplexBScan = createComplexData();

	DataHandle Amps = createData();
	DataHandle Phases = createData();

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	int AScansPerBScan = 1024;
	double LengthOfBScan = 2;
	ScanPatternHandle Pattern = createBScanPattern(Probe, LengthOfBScan, AScansPerBScan);

	startMeasurement(Dev, Pattern, Acquisition_AsyncContinuous);
	while (!_kbhit())
	{
		getRawData(Dev, Raw);
		// the required processing output for the standard processing routine is complex
		setComplexDataOutput(Proc, ComplexBScan);
		// the standard processing routines needs to be executed before
		executeProcessing(Proc, Raw);

		// specify the outputs for doppler processing
		setDopplerAmplitudeOutput(DopplerProc, Amps);
		setDopplerPhaseOutput(DopplerProc, Phases);


		// chose averaging parameter for the doppler processing routine
		setDopplerPropertyInt(DopplerProc, Doppler_Averaging_1, 3);
		setDopplerPropertyInt(DopplerProc, Doppler_Averaging_2, 3);

		// executes the doppler processing
		executeDopplerProcessing(DopplerProc, ComplexBScan);
	}
	stopMeasurement(Dev);

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	clearScanPattern(Pattern);

	clearData(Amps);
	clearData(Phases);
	clearComplexData(ComplexBScan);
	clearRawData(Raw);

	clearProcessing(Proc);
	clearDopplerProcessing(DopplerProc);
	closeProbe(Probe);
	closeDevice(Dev);

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}
}
	
void SpeckleVarianceOCT()
{
	// This demo program shows how to perform the additional processing used for Doppler OCT imaging.
	char message[1024];

	OCTDeviceHandle Dev = initDevice();
	ProbeHandle Probe = initProbe(Dev, "Probe");
	ProcessingHandle Proc = createProcessingForDevice(Dev);


	// chose parameter for oversampling
	int Oversampling_Slow_Axis = 2;
	setProbeParameterInt(Probe, Probe_Oversampling, 1);
	setProcessingParameterInt(Proc, Processing_SpectrumAveraging, 1);
	setProbeParameterInt(Probe, Probe_Oversampling_SlowAxis, Oversampling_Slow_Axis);

	int AScansPerBScan = 128;
	double LengthOfBScan = 2;
	int BScansPerVolume = 128;
	double WidthOfVolume = 2;
	ScanPatternHandle Pattern = createVolumePattern(Probe, LengthOfBScan, AScansPerBScan, WidthOfVolume, BScansPerVolume, ScanPattern_ApoEachBScan, ScanPattern_AcqOrderAll);

	RawDataHandle Raw = createRawData();
	ComplexDataHandle Volume = createComplexData();
	reserveComplexData(Volume, getDevicePropertyInt(Dev, DevicePropertyInt::Device_SpectrumElements) / 2, AScansPerBScan, BScansPerVolume);

	SpeckleVarianceHandle Var = initSpeckleVariance();

	DataHandle Mean = createData();
	DataHandle Variance = createData();

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	startMeasurement(Dev, Pattern, Acquisition_AsyncFinite);
	getRawData(Dev, Raw);
	// the required processing output for the standard processing routine is complex
	setComplexDataOutput(Proc, Volume);
	// the standard processing routines needs to be executed before
	executeProcessing(Proc, Raw);
	stopMeasurement(Dev);

	// chose averaging parameter for speckle variance processing
	setSpeckleVariancePropertyInt(Var, SpeckleVariancePropertyInt::SpeckleVariance_Averaging_1, 1);
	setSpeckleVariancePropertyInt(Var, SpeckleVariancePropertyInt::SpeckleVariance_Averaging_2, 2);

	computeSpeckleVariance(Var, Volume, Mean, Variance);

	// export data
	exportData(Variance, DataExportFormat::DataExport_CSV, "C:\\tmp\\Variance_test");

	closeSpeckleVariance(Var);

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	clearScanPattern(Pattern);

	clearData(Mean);
	clearData(Variance);
	clearComplexData(Volume);
	clearRawData(Raw);

	clearProcessing(Proc);
	closeProbe(Probe);
	closeDevice(Dev);

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}
}

void ExternalTriggerModus()
{
	// Please read the software and hardware manual carefully before using the external trigger mode in software. 
	// This demo program makes clear when the external trigger needs to be applied and when it needs to be turned off.
	// It is necessary to follow this instructions since additional measurements before the data acquisition itself need to be done without an underlying trigger signal
	// and the synchronization between the mirror(s) and the camera cannot be initiated correctly with external trigger signal applied.
	cout << "Do not trigger externally until the measurement is started correctly" << endl;
	char message[1024];

	OCTDeviceHandle Dev = initDevice();
	ProbeHandle Probe = initProbe(Dev, "Probe");
	ProcessingHandle Proc = createProcessingForDevice(Dev);

	RawDataHandle Raw = createRawData();
	DataHandle BScan = createData();

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	int AScansPerBScan = 1024;
	double LengthOfBScan = 2;
	ScanPatternHandle Pattern = createBScanPattern(Probe, LengthOfBScan, AScansPerBScan);

	ColoredDataHandle ColoredBScan = createColoredData();
	ColoringHandle Coloring = createColoring32Bit(ColorScheme_BlackAndWhite, Coloring_RGBA);
	setColoringBoundaries(Coloring, 0.0f, 70.0f);

	// setting the trigger mode to external and specify the timeout
	setTriggerMode(Dev, Trigger_External_AScan);
	setTriggerTimeout_s(Dev, 5);

	startMeasurement(Dev, Pattern, Acquisition_AsyncContinuous);
	cout << "External triggering possible as of now. " << endl;
	if (getError(message, 512))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	while (!_kbhit())
	{
		// if no trigger signal is applied the program will stuck in the function #getRawData until the timeout occurs
		getRawData(Dev, Raw);

		setProcessedDataOutput(Proc, BScan);
		executeProcessing(Proc, Raw);
	}

	colorizeData(Coloring, BScan, ColoredBScan, false);
	exportColoredData(ColoredBScan, ColoredDataExport_PNG, Direction::Direction_3, "ExtTriggerTestImg", 0);

	stopMeasurement(Dev);
	
	cout << "Please stop the external trigger signal here to make sure that the next measurement can be started correctly. " << endl;
	clearRawData(Raw);
	clearData(BScan);

	clearScanPattern(Pattern);
	closeProbe(Probe);
	clearProcessing(Proc);
	closeDevice(Dev);

	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}
}

void BatchMeasurementWithPolarizationAdjustment()
{
	// Some polarisation-sensitive OCT devices (at the time of writing only the VEGA 200 series) possess a motorized control stage which allows
	// the modification of the sampled polarisation.
	// This demo will capture a batch of B-Scans, each with a different setting of the polarisation control and export the resulting images to C:\OCTExport\demo_batch_*.png

	// Each polarization retarder will go through n POLARIZATION_STEPS, so overall n*n images will be captured
	const int POLARIZATION_STEPS = 4;
	const std::string EXPORT_FOLDER = "C:\\OCTExport";

	// initialization of device
	OCTDeviceHandle Dev = initDevice();

	// Check if device actually supports polarization adjustment
	if (!isPolarizationAdjustmentAvailable(Dev))
	{
		closeDevice(Dev);
		cout << "ERROR: " << "Device does not support polarization adjustment" << endl;
		_getch();
		return;
	}

	// Ensure export directory exists
	CreateDirectory(EXPORT_FOLDER.c_str(), NULL);

	// initialization of probe and processing handles
	ProbeHandle Probe = initProbe(Dev, "Probe");
	ProcessingHandle Proc = createProcessingForDevice(Dev);

	RawDataHandle Raw = createRawData();
	DataHandle BScan = createData();
	ColoringHandle Coloring = createColoring32Bit(ColorScheme_BlackAndWhite, Coloring_RGBA);
	setColoringBoundaries(Coloring, 0.0, 70.0);

	ScanPatternHandle Pattern = createBScanPattern(Probe, 2.0, 1024);
	
	startMeasurement(Dev, Pattern, Acquisition_Sync);

	for (int polHalfWave = 0; polHalfWave < POLARIZATION_STEPS; polHalfWave++)
	{
		setPolarizationAdjustmentRetardation(Dev, Retarder_Half_Wave, (double)polHalfWave / (POLARIZATION_STEPS - 1), Wait);
		for (int polQuarterWave = 0; polQuarterWave < POLARIZATION_STEPS; polQuarterWave++)
		{
			cout << "Capturing image " << polHalfWave << "_" << polQuarterWave << endl;
			// setPolarizationAdjustmentRetardation(Dev, Retarder_Quarter_Wave, (double)polQuarterWave / (POLARIZATION_STEPS - 1), Wait);
			// Workaround for ill-programmed servo in dev vega
			setPolarizationAdjustmentRetardation(Dev, Retarder_Quarter_Wave, (double)(polQuarterWave+1) / (POLARIZATION_STEPS), Wait);

			getRawData(Dev, Raw);
			setProcessedDataOutput(Proc, BScan);
			executeProcessing(Proc, Raw);

			std::ostringstream filename;
			filename << EXPORT_FOLDER << "\\demo_batch_" << polHalfWave << "_" << polQuarterWave << ".png";
			exportDataAsImage(BScan, Coloring, ColoredDataExport_PNG, Direction_3, filename.str().c_str(), ExportOption_DrawScaleBar | ExportOption_DrawMarkers | ExportOption_UsePhysicalAspectRatio);			
		}
	}

	stopMeasurement(Dev);

	clearScanPattern(Pattern);

	clearData(BScan);
	clearRawData(Raw);

	clearProcessing(Proc);
	closeProbe(Probe);
	closeDevice(Dev);

	// check if an error occured and write it to the command prompt
	char message[1024];
	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	// waits to close the command prompt until an input from the user is done
	_getch();
}

void AutomaticReferenceAndAmplificationAdjustment()
{
	// Some OCT devices (at the time of writing only the VEGA 200 series) possess a motorized reference intensity control stage which allows
	// the modification of the amount of reference light returned to the sensor. Additionally, the device may contain an amplification control stage,
	// which determines by how much the analog output of the sensor is amplified before being sampled and digitized. Both amplification and reference light
	// need to be adjusted together to ensure an optimal image.
	// The goal is to have both the reference light and the amplification as high as possible without oversaturating the digitization stage. The following code
	// attempts an automated adjustment of these two values. This highly depends on the concrete sample being imaged and may fail or yield sub-optimal results.
	// During the calibration process, a series of images will be written to C:\OCTExport, allowing a visual understanding of the process.

	// initialization of device
	OCTDeviceHandle Dev = initDevice();
	ProcessingHandle Proc = createProcessingForDevice(Dev);

	// Check if device has required hardware
	if (!isAmplificationControlAvailable(Dev) || !isReferenceIntensityControlAvailable(Dev) || !getProcessingFlag(Proc, Processing_CalculateSaturation))
	{
		closeDevice(Dev);
		clearProcessing(Proc);
		cout << "ERROR: " << "Device does not have required controls." << endl;
		_getch();
		return;
	}

	const std::string EXPORT_FOLDER = "C:\\OCTExport";
	// Ensure export directory exists
	CreateDirectory(EXPORT_FOLDER.c_str(), NULL);

	// initialization of probe and processing handles
	ProbeHandle Probe = initProbe(Dev, "Probe");

	RawDataHandle Raw = createRawData();
	DataHandle BScan = createData();
	ColoringHandle Coloring = createColoring32Bit(ColorScheme_BlackAndWhite, Coloring_RGBA);
	setColoringBoundaries(Coloring, 0.0, 70.0);

	ScanPatternHandle Pattern = createBScanPattern(Probe, 2.0, 1024);

	startMeasurement(Dev, Pattern, Acquisition_Sync);

	const int MaxAmplification = getAmplificationControlNumberOfSteps(Dev) - 1;
	// The processing step will calculate a saturation metric, which corresponds to the saturation of the sensor.
	// Ideally, this value should be around 80%
	double Saturation;
	
	// Amplification and Reference light intensity are the two parameters that this algorithm tries to optimize
	int Amplification;
	double Reference;
	bool Success = false;

	// This is not actually used as a loop. The construct just allows us to use break to abort the execution and jump to the cleanup stage.
	do
	{
		// Step 1: Set amplification and reference light to maximum (should oversaturate)
		setAmplificationControlStep(Dev, MaxAmplification);
		setReferenceIntensityControlValue(Dev, 1, Wait);

		getRawData(Dev, Raw);
		setProcessedDataOutput(Proc, BScan);
		executeProcessing(Proc, Raw);
		Saturation = getRelativeSaturation(Proc);

		std::ostringstream filename;
		filename << EXPORT_FOLDER << "\\demo_autoadjust_01_full_" << Saturation << ".png";
		exportDataAsImage(BScan, Coloring, ColoredDataExport_PNG, Direction_3, filename.str().c_str(), ExportOption_DrawScaleBar | ExportOption_DrawMarkers | ExportOption_UsePhysicalAspectRatio);

		if (Saturation < 0.2)
		{
			cout << "ERROR: Sensor did not reach saturation, even with full reference light and amplification. Please choose a different sample. Saturation was " << Saturation << endl;
			_getch();
			break;
		}

		// Step 2: Reduce amplification until under-saturation is reached. Then increase amplification by one step
		Amplification = MaxAmplification;

		while (Amplification > 0)
		{
			Amplification -= 1;
			setAmplificationControlStep(Dev, Amplification);

			getRawData(Dev, Raw);
			setProcessedDataOutput(Proc, BScan);
			executeProcessing(Proc, Raw);
			Saturation = getRelativeSaturation(Proc);

			std::ostringstream filename;
			filename << EXPORT_FOLDER << "\\demo_autoadjust_02_reduce_" << (Amplification+1) << "_" << Saturation << ".png";
			exportDataAsImage(BScan, Coloring, ColoredDataExport_PNG, Direction_3, filename.str().c_str(), ExportOption_DrawScaleBar | ExportOption_DrawMarkers | ExportOption_UsePhysicalAspectRatio);

			if (Saturation < 0.9)
			{
				Amplification += 1;
				break;
			}
		}

		setAmplificationControlStep(Dev, Amplification);

		// Step 3: Reduce reference light until proper saturation is reached. This algorithm uses a binary search algorithm to find the optimum.
		double ReferenceLow = 0;
		double ReferenceHigh = 1;

		while (ReferenceHigh - ReferenceLow > 0.01)
		{
			Reference = (ReferenceLow + ReferenceHigh) / 2;
			setReferenceIntensityControlValue(Dev, Reference, Wait);

			getRawData(Dev, Raw);
			setProcessedDataOutput(Proc, BScan);
			executeProcessing(Proc, Raw);
			Saturation = getRelativeSaturation(Proc);

			std::ostringstream filename;
			filename << EXPORT_FOLDER << "\\demo_autoadjust_03_setint_ref_" << Reference << "_" << Saturation << ".png";
			exportDataAsImage(BScan, Coloring, ColoredDataExport_PNG, Direction_3, filename.str().c_str(), ExportOption_DrawScaleBar | ExportOption_DrawMarkers | ExportOption_UsePhysicalAspectRatio);

			if (Saturation < 0.75)
				ReferenceLow = Reference;
			else if (Saturation >= 0.80)
				ReferenceHigh = Reference;
			else
				break;
		}
		Success = true;
	} while (false);
	stopMeasurement(Dev);

	clearScanPattern(Pattern);

	clearData(BScan);
	clearRawData(Raw);

	clearProcessing(Proc);
	closeProbe(Probe);
	closeDevice(Dev);

	// check if an error occured and write it to the command prompt
	char message[1024];
	if (getError(message, 1024))
	{
		cout << "ERROR: " << message << endl;
		_getch();
		return;
	}

	if (Success)
		cout << "Algorithm complete. Final saturation: " << Saturation << ". Parameters: Amplification: " << (Amplification+1) << "/" << (MaxAmplification+1) << " Reference Light Intensity: " << Reference << endl;

	// waits to close the command prompt until an input from the user is done
	_getch();
}

void ImageFieldCalibration()
{
	int AScansPerBScan = 256;
	int BScansPerVolume = 256;
	int AScanAveraging = 10;

	char errorMessage[1024];

	OCTDeviceHandle Dev = initDevice();
	ProbeHandle Probe = initProbe(Dev, "Probe");
	ProcessingHandle Proc = createProcessingForDevice(Dev);

	RawDataHandle Raw = createRawData();
	DataHandle Volume = createData();
	DataHandle Surface = createData();
	ImageFieldHandle ImageField = createImageField();

	if (getError(errorMessage, 1024))
	{
		cout << "ERROR: " << errorMessage << endl;
		_getch();
		return;
	}

	int maxRangeX = getProbeParameterFloat(Probe, ProbeParameterFloat::Probe_RangeMaxX);
	int maxRangeY = getProbeParameterFloat(Probe, ProbeParameterFloat::Probe_RangeMaxY);

	setProbeParameterInt(Probe, ProbeParameterInt::Probe_Oversampling, AScanAveraging);
	setProcessingParameterInt(Proc, ProcessingParameterInt::Processing_AScanAveraging, AScanAveraging);

	ScanPatternHandle Pattern = createVolumePattern(Probe, maxRangeX, AScansPerBScan, maxRangeY, BScansPerVolume, ScanPatternApodizationType::ScanPattern_ApoEachBScan, ScanPatternAcquisitionOrder::ScanPattern_AcqOrderAll);


	startMeasurement(Dev, Pattern, Acquisition_AsyncFinite);
	getRawData(Dev, Raw);
	setProcessedDataOutput(Proc, Volume);
	executeProcessing(Proc, Raw);
	stopMeasurement(Dev);

	// image field correcction
	determineSurface(Volume, Surface);
	determineImageField(ImageField, Pattern, Surface);

	setImageFieldInProbe(ImageField, Probe);
	saveProbe(Probe, "Probe");



	if (getError(errorMessage, 1024))
	{
		cout << "ERROR: " << errorMessage << endl;
		_getch();
		return;
	}

	clearScanPattern(Pattern);

	clearData(Volume);
	clearData(Surface);
	clearRawData(Raw);

	clearImageField(ImageField);

	clearProcessing(Proc);
	closeProbe(Probe);
	closeDevice(Dev);

	if (getError(errorMessage, 1024))
	{
		cout << "ERROR: " << errorMessage << endl;
		_getch();
		return;
	}
}

int main(){
	cout << "The following demonstration pograms are available: " << endl;
	cout << "a: How to read an .oct-file \n";
	cout << "b: How to write an .oct-file \n";
	cout << "c: Single steps of the processing chain \n";
	cout << "d: Creating one scan pattern out of several B-scans, e.g. rotating B-scans around the center \n";
	cout << "e: Handling of freeform scan patterns \n";
	cout << "f: Speed up the acquisition by modifying the scan pattern \n";
	cout << "g: Doppler OCT with the SDK \n";
	cout << "h: Speckle variance OCT with the SDK \n";
	cout << "i: External A-scan trigger (requires additional hardware) \n";
	cout << "j: Capture batch of images and adjust polarization retardation (requires supported device) \n";
	cout << "k: Auto-set amplification and reference light intensity (requires supported device) \n";
	cout << "l: Performs image field calibration \n";

	char c = _getch();
	switch (c)
	{
	case 'a': ReadOCTFile();
		break;
	case 'b': WriteOCTFile();
		break;
	case 'c': ProcessingChain();
		break;
	case 'd': AdvancedModificationOfScanPattern();
		break;
	case 'e': FreeformScanPatterns();
		break;
	case 'f': RemovingApoFromScanPattern();
		break;
	case 'g': DopplerOCT();
		break;
	case 'h': SpeckleVarianceOCT();
		break;
	case 'i': ExternalTriggerModus();
		break;
	case 'j': BatchMeasurementWithPolarizationAdjustment();
		break;
	case 'k': AutomaticReferenceAndAmplificationAdjustment();
		break;
	case 'l': ImageFieldCalibration();
		break;
	}
	_getch();
	return 0;
}