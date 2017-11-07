% Main script. The current configuration corresponds to the main simulation in:
% Masquelier T, Guyonneau R, Thorpe SJ (2009). Competitive STDP-based Spike Pattern Learning. Neural Computation: in press.
% Feel free to use and modify but please cite us.
% The main function is the mex file STDPContinuous.
% The script prepares the spike trains and the neurons, calls STDPContinuous and analyzes the results.
% timothee.masquelier@alum.mit.edu

% run ../../setPath

if ~exist('PARAM','var')
    global PARAM
end

param


if ~PARAM.goOn
    
    timedLogLn(['RANDOM STATE = ' int2str(PARAM.randomState) ]);
    
    % generate spike train
%     rand('state',PARAM.randomState);
%     randn('state',PARAM.randomState);    
    if exist(['../mat/afferent.rand' sprintf('%03d',PARAM.randomState) '.mat'],'file')
        load(['../mat/afferent.rand' sprintf('%03d',PARAM.randomState) '.mat'])
%         if ~exist('spikeList','var') % 'false' file
%             [spikeList afferentList] = generateSpikeTrain;
%             save(['../mat/afferent.rand' sprintf('%03d',PARAM.randomState) '.mat'],'spikeList','afferentList')
%         end
    else
        timedLogLn('Generating spike train...');   
        if PARAM.realValuedPattern
            [spikeList afferentList patternPeriod, values, times] = generateSpikeTrainWithRealValuedPattern2;
            save(['../mat/afferent.rand' sprintf('%03d',PARAM.randomState) '.mat'],'spikeList','afferentList','patternPeriod','values','times')
        else
            [spikeList afferentList] = generateSpikeTrain;
            save('-v7.3',['../mat/afferent.rand' sprintf('%03d',PARAM.randomState) '.mat'],'spikeList','afferentList')
        end
            
    end
    
    % init neuron
    N = round(3.5*PARAM.epspCut*PARAM.tm*length(spikeList)/spikeList(end));
    for nn=1:PARAM.nNeuron %neuron loop
        neuron(nn) = createNewNeuron(PARAM,N);
    end %neuron loop
end



% PARAM.threshold = Inf;
% stop = 0;
for r=1:PARAM.nRun
    tic

    n=n+1;

    timedLogLn(['Run ' int2str(n) ' (' int2str(length(spikeList)) ' iterations ~ ' int2str(length(spikeList)/25e6*length(neuron)*1e-3/PARAM.tmpResolution) ' min )'])

%     t=(n-1)*PARAM.T; % simulation time
    if n>1
        spikeList = spikeList+PARAM.T;% shift spike list

        maxLastFiring = 0;
        for nn=1:length(neuron)
            if neuron(nn).nFiring>0 && neuron(nn).firingTime(neuron(nn).nFiring)>maxLastFiring
                maxLastFiring = neuron(nn).firingTime(neuron(nn).nFiring);
            end
        end
        spikeList = max(spikeList,maxLastFiring); % this is to avoid inserting epsp before last firing
        %         for n=1:length(neuron) %neuron loop
        %             neuron(n).nextFiring = Inf;
        %         end
        for nn=1:length(neuron) % make sure epsp are chronologic
            neuron(nn).nEpsp=0;
            neuron(nn).maxPotential=0;
            if PARAM.fixedFiringMode
                neuron(nn).nextFiring = (n-1)*PARAM.T + PARAM.fixedFiringLatency;
            else
                neuron(nn).nextFiring = Inf;
            end
        end
    end
    
    if PARAM.dump


%         % beginning
%         nSpike = round(1/3/PARAM.T*length(spikeList));
%         delete('dump.txt');
%         neuron=STDPContinuous(neuron,spikeList(1:nSpike),afferentList(1:nSpike)-1,true,false,PARAM);
%         copyfile('dump.txt','dump.beginning.txt')
%         break % no need to go further

        % end
        if r==PARAM.nRun
            nSpike = round((PARAM.T-1)/PARAM.T*length(spikeList));
            neuron=STDPContinuous(neuron,spikeList(1:nSpike),afferentList(1:nSpike)-1,false,PARAM.beSmart,PARAM);
            delete('dump.txt');
            neuron=STDPContinuous(neuron,spikeList(nSpike+1:end),afferentList(nSpike+1:end)-1,true,false,PARAM);
            copyfile('dump.txt','dump.end.txt')
        else
            neuron=STDPContinuous(neuron,spikeList,afferentList-1,false,PARAM.beSmart,PARAM); % C indexes start at 0
        end
        
    else
        neuron=STDPContinuous(neuron,spikeList,afferentList-1,false,PARAM.beSmart,PARAM); % C indexes start at 0
    end
    
%     % add new 'virgin' neurons
%     if r<PARAM.nRun-1
%         for nn=1:PARAM.nNeuron %neuron loop
%             neuron = [ neuron createNewNeuron(PARAM,N) ];
%         end
%     end
    
    disp(' ');
    toc
    
    if sum([neuron.nFiring]) == 0
    	warning('Neurons do not fire')
        break;
    end
            
end % run loop

if max([neuron.nFiring])>length(neuron(1).firingTime)
    warning('Increase firingTime array size')
end

% % save
% if PARAM.fixedFiringMode
%     weight = neuron.weight;
%     save('../mat/weight.mat','weight');
% end

% save all
c = clock;
timeTag = [sprintf('%02.0f',c(4)) '.' sprintf('%02.0f',c(5)) '.' sprintf('%02.0f',c(6)) ];


% perf;
% for nn=1:length(neuron)
%     timeTag = [timeTag '.n' sprintf('%02.0f',nn) '.HR' sprintf('%03.0f',100*neuron(nn).HR) '.FA' sprintf('%03.2f',neuron(nn).FA)  ];
% end

% if stop
%     timeTag = [timeTag '.S'];
% end
% timeTag = [timeTag '.nF' int2str(neuron.nFiring)];

% dispMultiPattern
multiPattern
writeLatencies


% tags useful for multi patterns
hr = -sort(-max(HR,[],2));
timeTag = [timeTag '.pat'];
for pat=1:PARAM.nPattern
    timeTag = [timeTag '.' sprintf('%03.0f',100*hr(pat)) ];
end
hr = -sort(-max(HR,[],1));
timeTag = [timeTag '.neur'];
for neur=1:length(neuron)
    timeTag = [timeTag '.' sprintf('%03.0f',100*hr(neur)) ];
end

% tags useful for mono pattern
% timeTag = [timeTag '.HR.' sprintf('%03.0f',100*HR) '.FA.' sprintf('%f',FA) ];


clear spikeList
clear afferentList
clear patternPeriod
clear values
clear times
% save(['../mat/matlab.rand' sprintf('%03d',PARAM.randomState) '.' timeTag '.mat']);
disp(timeTag)
save

if PARAM.fixedFiringMode
    weight = neuron.weight;
    save('../mat/weight.mat','weight');
end

if PARAM.dump
    showPot
end

dispMultiPattern

% displayResult
% perf

