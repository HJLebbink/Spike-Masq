
multiPattern

f = figure('Name','Overview');
co = get(gca,'ColorOrder');

nR = PARAM.nPattern;
nC = length(neuron);
margin = .25;
height = nR + (nR+1)*margin + 1;
width = nC + (nC+1)*margin + 1;

for neur=1:length(neuron)

%     figure('Name',['Neuron ' int2str(neur) ' - final weight distribution - ' timeTag])
%     hist(neuron(neur).weight)
%     legend(['Weight sum = ' num2str(sum(neuron(neur).weight))])
    
    figure(f)
    
    for pat=1:PARAM.nPattern

%         subplot(PARAM.nPattern,PARAM.nNeuron,neur+(pat-1)*PARAM.nNeuron)
        subplot('Position',[(neur*(1+margin))/width (margin+(nR-pat)*(1+margin))/height 1/width 1/height])
        if ~isempty(latency)
            plot(1000*latency{pat,neur},'.','MarkerSize',5,'MarkerEdgeColor','k','MarkerFaceColor','k')
            axis([0 length(latency{pat,neur}) 0 1000*PARAM.copyPasteDuration])
            title(['HR=' int2str(100*HR(pat,neur)) '% - FA=' sprintf('%.1f',FA(pat,neur)) 'Hz'],'FontSize',8)
        end

        if pat==PARAM.nPattern
            xlabel('# discharges','FontSize',8)
        end
        if neur==1
            ylabel('Latency (ms)','FontSize',8)
        end
        
        set(gca,'FontSize',8)

    end % pat
end % neur

% % color patches
% patchSize = .5;
% for i=1:PARAM.nPattern
%     subplot('Position',[(.5-patchSize/2)/width ((.5-patchSize/2)+margin+(nR-i)*(1+margin))/height patchSize/width patchSize/height])
%     rectangle('Position',[0 0 1 1],'FaceColor',(.5*co(1+mod(1+i,7),:)+.5*zeros(1,3)),'EdgeColor','none')
%     axis off
% end
% for j=1:length(neuron)
%     subplot('Position',[ ((.5-patchSize/2)+j*(1+margin))/width 1-(.5+patchSize/2)/height  patchSize/width patchSize/height])
%     rectangle('Position',[0 0 1 1],'FaceColor',(.33*co(1+mod(1+j,7),:)+.67*ones(1,3)),'EdgeColor','none')
%     axis off
% end

% print -deps latencies.eps
% [result,msg] = eps2pdf('latencies.eps','C:\Archivos de programa\gs\gs8.54\bin\gswin32c.exe',0)

