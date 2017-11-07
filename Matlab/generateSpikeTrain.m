function [spikeList, afferentList]=generateSpikeTrain

global PARAM

rand('state',PARAM.randomState);
randn('state',PARAM.randomState);    

% nSpikePatternTotal = 0;
nNoSpike = 0;

spikeList = [];
afferentList = [];
spontaneousActivity = [];
pattern={};
% patternTime=[];

data = zeros(1,PARAM.nCopyPasteAfferent);

nNormal = 0;
nSpontaneous = 0;


if PARAM.spontaneousActivity > 0
    timedLog(['Adding spontaneous activity for neuron ', int2str(a)])
    for a=1:PARAM.nAfferent
        spontaneousActivity = variablePoissonSpikeTrain(PARAM.spontaneousActivity,PARAM.spontaneousActivity,0,PARAM.T,PARAM.dt,Inf);
        %         spontaneousActivity = variablePoissonSpikeTrain(0,PARAM.spontaneousActivity,PARAM.spontaneousActivity/PARAM.copyPasteDuration,PARAM.T,PARAM.dt,PARAM.maxTimeWithoutSpike);
        nSpontaneous = nSpontaneous + length(spontaneousActivity);

        spikeList = [spikeList  spontaneousActivity];
        afferentList = [afferentList uint16(a*ones(1,length(spontaneousActivity)))];
    end
end

timedLog(['Main spike train'])
% rate = PARAM.maxFiringRate*rand(1,PARAM.nAfferent);
for a=1:PARAM.nAfferent
    if mod(a,100)==0
        timedLog([int2str(a) ' afferents done'])
    end
    %     afferent(a).rate = rate(a);
    %     afferent(a).spikeTrain = poissonSpikeTrain(rate(a),PARAM.T,PARAM.dt);
    if PARAM.oscillations
        sl = oscillatorySpikeTrain(PARAM.maxFiringRate/2,1/PARAM.copyPasteDuration,PARAM.T,PARAM.dt);
    else
        sl = variablePoissonSpikeTrain(0,PARAM.maxFiringRate,PARAM.maxFiringRate/PARAM.copyPasteDuration,PARAM.T,PARAM.dt,PARAM.maxTimeWithoutSpike);
    end
    %     sl = variablePoissonSpikeTrain(50,50,0,PARAM.T,PARAM.dt,PARAM.maxTimeWithoutSpike);

    nNormal = nNormal+length(sl);

    for pat=1:PARAM.nPattern
        pattern={};
        if (PARAM.nPattern==1   &&  a <= PARAM.nCopyPasteAfferent ) || ... % with just one pattern, afferents are numbered so that the indices < nCopyPasteAfferent are involved in the pattern
           (PARAM.nPattern>1    &&  rand<PARAM.nCopyPasteAfferent/PARAM.nAfferent )
            [sl toCopy{a}] = copyDeletePaste(sl,PARAM.jitter,pat);
            %         nNoSpike = nNoSpike + (length(toCopy{a})==0);
            %         nSpikePatternTotal = nSpikePatternTotal+length(toCopy);
%             data(a) = length(toCopy{a});

        end
    end
    spikeList = [spikeList sl ];
    afferentList = [afferentList uint16(a*ones(1,length(sl)))];
end


% error('s')

% % paste (eventually incomplete in case of deletion)
% timedLog(['Pasting the pattern'])
% nToDelete = round(PARAM.spikeDeletion*length(pattern));
% for p=1:length(PARAM.posCopyPaste)
%     rp = randperm(length(pattern));
%     for s=1:length(rp)-nToDelete
%         n=n+1;
%         a=str2num(pattern{rp(s)}(1:5));
%         i=str2num(pattern{rp(s)}(7:end));
%         spikeList(n) = (PARAM.posCopyPaste(p)-1)*PARAM.copyPasteDuration+applyJitter(toCopy{a}(i),PARAM.jitter);
%         afferentList(n) = a;
%     end
%     % invent random spikes (to maintain stable density)
%     for s=1:nToDelete
%         n=n+1;
%         a = ceil(rand*PARAM.nCopyPasteAfferent);
%         time = rand*PARAM.copyPasteDuration;
%         spikeList(n) = (PARAM.posCopyPaste(p)-1)*PARAM.copyPasteDuration+time;
%         afferentList(n) = a;
%     end
% end

% % tmp
% figure('Name','Distribution of # spikes in pattern')
% hist(data,max(data)+1);

% % reorder
% oldPos = [1:PARAM.nCopyPasteAfferent];
% add = [];
% cursor = 1;
% while true
%     if data(oldPos(cursor))==0
%         add = [ add oldPos(cursor) ];
%         oldPos(cursor) = [];
%     else
%         cursor = cursor+1;
%     end
%     if cursor>length(oldPos)
%         break
%     end
% end
% oldPos = [oldPos add];
% [tmp idx] = sort(oldPos);
% 
% for s=1:length(afferentList)
%     if afferentList(s) <= PARAM.nCopyPasteAfferent
%         afferentList(s) = idx( afferentList(s) );
%     end
% end
% % PARAM.nCopyPasteAfferent = PARAM.nCopyPasteAfferent - sum(data==0);


% sort both lists
[spikeList idx] = sort(spikeList);
tmp = uint16(zeros(1,length(spikeList)));
for s=1:length(spikeList)
    tmp(s) = afferentList(idx(s));
end
afferentList = tmp;

disp([int2str(length(spikeList)) ' iterations'])
disp(['Spontaneous activity: ' num2str(100*nSpontaneous/(nNormal+nSpontaneous)) '%' ])
% disp([int2str(length(pattern)) ' spikes in copy-pasted pattern.'])
% disp(['About ' int2str(length(pattern)*PARAM.tm/PARAM.copyPasteDuration) ' copy-pasted spikes in effective integration time window.'])
% disp(['(this should be > threshold=' num2str(PARAM.threshold) ')'])
% disp([int2str(nNoSpike) ' afferent with no spike in copy-pasted pattern.'])

function [spikeList, toCopy] = copyDeletePaste(spikeList,jitter,pat)

global PARAM

toCopy = [];
toRemove = [];

s=1;
for p=1:length(PARAM.posCopyPaste{pat})
    from = (PARAM.posCopyPaste{pat}(p)-1)*PARAM.copyPasteDuration;
    to = from + PARAM.copyPasteDuration;
    %     center = (PARAM.posCopyPaste{pat}(p)-1+.5)*PARAM.copyPasteDuration;
    while s<=length(spikeList) && spikeList(s)<from
        s = s+1;
    end
    while s<=length(spikeList) && spikeList(s)<=to
        if p==5
            toCopy = [toCopy spikeList(s)-(PARAM.posCopyPaste{pat}(p)-1)*PARAM.copyPasteDuration];%copy
        end
        toRemove = [toRemove s]; %flag to suppress spike
        s = s+1;
    end
end

% nSpikePattern = length(toCopy);



% if nSpikePattern>0 % only include afferent that do spike in pattern
% suppress
spikeList(toRemove)=[];

% % paste (now paste is done in the main function)
cursor = length(spikeList);
spikeList(length(spikeList)+length(PARAM.posCopyPaste{pat})*length(toCopy))=0;
for p=1:length(PARAM.posCopyPaste{pat})
    spikeList(cursor+1:cursor+length(toCopy)) = [ (PARAM.posCopyPaste{pat}(p)-1)*PARAM.copyPasteDuration+applyJitter(toCopy,jitter) ];
    cursor = cursor+length(toCopy);
end
% end
% error('stop')

function spikeList=applyJitter(spikeList,jitter)
% if jitter~=0
for i=1:length(spikeList)
    spikeList(i) = spikeList(i)+jitter*randn;
end
% end
