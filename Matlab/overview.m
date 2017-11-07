figure('Name',['spikeTrain - ' timeTag])
for i=1:3

%     subplot(3,1,i)
    rectangle('Position',[ 0 0 max([neuron.epspTime]) PARAM.nAfferent],'FaceColor',0*ones(1,3),'EdgeColor','none')

    co = get(gca,'ColorOrder');
    
    for pat=1:PARAM.nPattern
        for p=1:length(PARAM.posCopyPaste{pat})
            rectangle('Position',[(n-1)*PARAM.T+(PARAM.posCopyPaste{pat}(p)-1)*PARAM.copyPasteDuration 0 PARAM.copyPasteDuration PARAM.nAfferent],'FaceColor',(.5*co(1+mod(1+pat,7),:)+.5*zeros(1,3)),'EdgeColor','none')
            %     plot((PARAM.posCopyPaste(p)-1)*PARAM.copyPasteDuration*ones(1,2),[0 PARAM.nCopyPasteAfferent],'LineWidth',1)
            %     plot(PARAM.posCopyPaste(p)*PARAM.copyPasteDuration*ones(1,2),[0 PARAM.nCopyPasteAfferent],'LineWidth',1)
            hold on
        end
    end
    
    
    for nn=1:length(neuron)
        for f=1:neuron(nn).nFiring
            if neuron(nn).firingTime(f)>=(n-1)*PARAM.T
                plot(neuron(nn).firingTime(f)*ones(1,2),[0 PARAM.nAfferent],'LineWidth',1.5,'Color',.33*co(1+mod(1+nn,7),:)+.67*ones(1,3))
                hold on
            end
        end
%        leg{nn} = ['neuron #' int2str(nn)];
    end
%     if i==1
%         legend(leg)
%     end

    %     % spikes
    %     if i>1
    %         if i==2
    %             range=[(n-1)*PARAM.T ((n-1)+1/100)*PARAM.T];
    %         else
    %             range=[PARAM.T*(n-1/100) PARAM.T*n 0 PARAM.nAfferent];
    %         end
    %         clear afferent
    %         afferent{PARAM.nAfferent} = [];
    %         for s=length(spikeList):-1:1
    %             if spikeList(s)>range(2)
    %                 continue
    %             else
    %                 if spikeList(s)<range(1)
    %                     break
    %                 end
    %             end
    %             afferent{afferentList(s)} = [ afferent{afferentList(s)} spikeList(s)];
    %             %         plot(spikeList(s),afferentList(s),'.','MarkerSize',5,'MarkerEdgeColor',neuron.weight(afferentList(s))*[1 0 0]+(1-neuron.weight(afferentList(s)))*[1 1 1]);
    %         end
    %         for a=1:PARAM.nAfferent
    %             plot(afferent{a},a*ones(1,length(afferent{a})),'.','MarkerSize',5,'MarkerEdgeColor',neuron.weight(a)*[1 0 0]+(1-neuron.weight(a))*[1 1 1]);
    %         end
    %         clear afferent
    %     end

    xlabel('t (s)')
    ylabel('afferents')
    switch i
        case 1
            axis([(n-1)*PARAM.T n*PARAM.T 0 PARAM.nAfferent])
            title('All')
        case 2
            axis([(n-1)*PARAM.T ((n-1)+2/100)*PARAM.T 0 PARAM.nAfferent])
            title('Begining')
        case 3
            axis([PARAM.T*(n-2/100) PARAM.T*n 0 PARAM.nAfferent])
            title('End')
    end
end