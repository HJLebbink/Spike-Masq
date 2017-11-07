disp('Latencies:')
disp(num2str(lat))
tmp = sort(lat,2);
disp('Latencies difference:')
fid=fopen('latencyDiff.csv','a');
for pat=1:PARAM.nPattern
    fprintf(1,'Pattern #%d\n',pat),
    for neur=2:length(neuron)
        if tmp(pat,neur-1)>0 % valid latency difference
            fprintf(1,'%f ',tmp(pat,neur)-tmp(pat,neur-1)),
            fprintf(fid,'%f;%f\n',PARAM.inhibStrength,tmp(pat,neur)-tmp(pat,neur-1));
        end
    end
    fprintf(1,'\n'),
end
returnCode = fclose(fid);
