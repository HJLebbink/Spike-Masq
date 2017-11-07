
clear latency

range = n*PARAM.T + [ -PARAM.T 0 ];

latency{PARAM.nPattern,length(neuron)}={};

HR = zeros(PARAM.nPattern,length(neuron));
FA = zeros(PARAM.nPattern,length(neuron));
lat = zeros(PARAM.nPattern,length(neuron));


for neur=1:length(neuron)
    
    for pat=1:PARAM.nPattern
    
        latency{pat,neur} = [];
        for f=1:neuron(neur).nFiring
            nn = ceil(neuron(neur).firingTime(f)/PARAM.T);
        %     if neuron(neur).firingTime(f)>=(nn-1)*PARAM.T
            period = ceil((neuron(neur).firingTime(f)-(nn-1)*PARAM.T)/PARAM.copyPasteDuration);
            if (binarySearch(PARAM.posCopyPaste{pat},period,true) > 0) %hit
                latency{pat,neur} = [ latency{pat,neur} neuron(neur).firingTime(f)-(nn-1)*PARAM.T - (period-1)*PARAM.copyPasteDuration];
            else
                latency{pat,neur} = [latency{pat,neur} 0]; % convention
            end
            %     end
        end
        
        goodIndices = neuron(neur).firingTime(1:neuron(neur).nFiring)>=range(1) & neuron(neur).firingTime(1:neuron(neur).nFiring)<range(2);
        
        if ~isempty(goodIndices)        
            hit = sum( latency{pat,neur} .* goodIndices >0);
            HR(pat,neur) = hit/sum((PARAM.posCopyPaste{pat}-1)*PARAM.copyPasteDuration+(n-1)*PARAM.T>=range(1) & (PARAM.posCopyPaste{pat}-1)*PARAM.copyPasteDuration+(n-1)*PARAM.T<range(2));
            FA(pat,neur) = (sum(goodIndices) - hit)/(range(2)-range(1));

            if ((HR(pat,neur) > .9) && (FA(pat,neur)<1))% arbitrary criterion
                lat(pat,neur) = meanNonZero( latency{pat,neur} .* goodIndices );
            end
        end
        
    end % pat
end % neur




% f = figure('Name','Overview');
% 
% for neur=1:length(neuron)
% 
% %     figure('Name',['Neuron ' int2str(neur) ' - final weight distribution - ' timeTag])
% %     hist(neuron(neur).weight)
% %     legend(['Weight sum = ' num2str(sum(neuron(neur).weight))])
%     
%     figure(f)
%     
%     for pat=1:PARAM.nPattern
% 
%         subplot(PARAM.nPattern,PARAM.nNeuron,neur+(pat-1)*PARAM.nNeuron)
%         if ~isempty(latency)
%             plot(1000*latency{pat,neur},'.','MarkerSize',5)
%             axis([0 length(latency{pat,neur}) 0 1000*PARAM.copyPasteDuration])
%             title(['HR=' num2str(100*HR(pat,neur)) '% - FA=' num2str(FA(pat,neur)) 'Hz'],'FontSize',8)
%         end
% 
%         xlabel('# discharges','FontSize',8)
%         ylabel('Latency (ms)','FontSize',8)
% 
%     end % pat
% end % neur
