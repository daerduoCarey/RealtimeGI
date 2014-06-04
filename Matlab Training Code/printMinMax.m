function rst = printMinMax(netFile, i)
    load(netFile);
    inMin = net.inputs{1}.processSettings{2}.xmin
    inMax = net.inputs{1}.processSettings{2}.xmax
    outMin = net.outputs{3}.processSettings{1}.xmin
    outMax = net.outputs{3}.processSettings{1}.xmax
    FileName = strcat(num2str(i), '_minmax.txt');
    dlmwrite(FileName, inMin, '-append');
    dlmwrite(FileName, inMax, '-append');
    dlmwrite(FileName, outMin, '-append');
    dlmwrite(FileName, outMax, '-append');
end