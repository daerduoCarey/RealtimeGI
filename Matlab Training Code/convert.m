% convert
function rst = convert(netfile, objID)
    load(netfile);
    
    IWfile = strcat(num2str(objID), 'IW.txt');
    dlmwrite(IWfile, net.IW{1}, ',');
    
    LWfile1 = strcat(num2str(objID), 'LW1.txt');
    dlmwrite(LWfile1, net.LW{2,1}, ',');
    
    LWfile2 = strcat(num2str(objID), 'LW2.txt');
    dlmwrite(LWfile2, net.LW{3,2}, ',');
    
    Bfile1 = strcat(num2str(objID), 'B1.txt');
    dlmwrite(Bfile1, net.b{1}', ',');
    
    Bfile1 = strcat(num2str(objID), 'B2.txt');
    dlmwrite(Bfile1, net.b{2}', ',');
    
    Bfile1 = strcat(num2str(objID), 'B3.txt');
    dlmwrite(Bfile1, net.b{3}', ',');
end

    