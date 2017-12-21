% Zachary Olech, zolech
% 12/13/2017

%% Clear Environment
clear;
clc;

%% Text and Labels
tempGraphLabels = {'Degrees (Celcius)', 'Resistance (Ohms)'};

%% Variables
inputFile = 'DATALOG.txt';%input('Please enter the name of the file that you would like to read: ','s');   <-- Fix this eentually
formatSpec = '%d %s %f %f';
delimiter = ',';
loopLength = 1;    %~1 Second Per Loop NOT EXACT TIME AS OF NOW

%% Read Input
fileID = fopen(inputFile, 'r', 'n', 'ISO-8859-15');
dataIn = textscan(fileID,formatSpec,'Delimiter',delimiter);
rowNum = length(dataIn{1,1});

%% Process the Data
% Calculate Time
timeTable = zeros(rowNum,4);
timeTable(:,1) = dataIn{1,1}(:,1);          % Index
timeTable(:,2) = timeTable(:,1)*loopLength; % Time of system running in seconds
timeTable(:,3) = timeTable(:,2)/60;         % Time of system running in minutes
timeTable(:,4) = timeTable(:,3)/60;         % Time of system running in hours

%Temperature Data
tempTable = zeros(rowNum,5);
tempTable(:,1) = dataIn{1,1}(:,1);          % Index
%tempTable(:,2) = dataIn{1,2}(:,1);         % Probe Number <- This is broken
tempTable(:,3) = dataIn{1,3}(:,1);          % Voltage Value
tempTable(:,4) = dataIn{1,4}(:,1);          % Resistance Value
% Calculate All Temperatures
tempTable(:,5) = calcTemp(tempTable(:,4));

%RPM Data
%primaryRPMTable = {1..2,:};
%secondaryRPMTable = {1,:};

%% Display Output
figure('Name', 'Car Data');

% Temperature Probe 0
p=plot(tempTable(:,1),tempTable(:,5));
xlabel(tempGraphLabels(1,1));
ylabel(tempGraphLabels(1,2));
p(1).LineWidth = 1;

% Extra space

%% Functions
% Calculate the temperatures from the temperature probe
function temp = calcTemp( r )

% Values from: http://www.bapihvac.com/wp-content/uploads/2010/11/Thermistor_100K.pdf
refTempTable = [ %R Ohms, T Farenheit
   8783, 187;
   11949, 171;
   14584, 161;
   17907, 151;
   22111, 141;
   27481, 131;
   34376, 121;
   43273, 111;
   54878, 101;
   70076, 91;
   90208, 81;
   117000, 71;
   153092, 61;
   201971, 51;
   269035, 41;
   361813, 31;
   491217, 21;
   674319, 11;
   935383, 1;
   1000019, -1;
];

% Calculate Temperatures
for t=1:1:length(r)
    i = 1;
    while r(t,1) > refTempTable(i,1)
        i = i + 1;
    end
    
    % P1(rl, th) P2(rh,tl)
    % Points
    rl = refTempTable(i,1);
    th = refTempTable(i+1,2);
    tl = refTempTable(i,2);
    rh = refTempTable(i+1,1);
    % Slope (Degrees/Resistance)
    m = (th-tl)/(rl-rh);
    % Using P1(rl, th)
    b = th-(m*rl);
    % Final Data
    temp(t,1) = m*r(i,1)+b;
end

end