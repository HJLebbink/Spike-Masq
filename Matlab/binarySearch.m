function idx=binarySearch(tab,val,isNum)

% returns the col index of val in sorted table tab

% returns 0 if val is not found

[nrows,ncols]=size(tab);

lo=1;

hi=ncols;

if isNum
    while (lo <= hi)
        idx = fix((lo+hi)/2);    
        if (val < tab(1,idx))
            hi = idx - 1;
        elseif (val > tab(1,idx))
            lo = idx + 1;
        else
            return;
        end;
    end;
else
    while (lo <= hi)
        idx = fix((lo+hi)/2); 
        if Strcmpc(char(val),char(tab(1,idx)))<0
            hi = idx - 1;
        elseif Strcmpc(char(val),char(tab(1,idx)))>0
            lo = idx + 1;
        else
            return;
        end;
    end;    
end

idx=0;

return;
