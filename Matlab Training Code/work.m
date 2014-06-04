% nn s

function ha = work(i)  
    filename = strcat('objData', num2str(i), '.txt');
    net = trainNNs(filename);
    
    netname = strcat('net', num2str(i), '.net');
    save(netname, 'net', '-mat');
end