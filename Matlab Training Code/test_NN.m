function heng = test_NN(t_in, t_out, net)
    [InputDim, InputSize] = size(t_in);
    OutputDim = 3;
    for i = 1:InputSize
        input = t_in(:, i);
        output = t_out(:,i);
        disp('compare:');
        cal_output = net(input);
        disp(output');
        disp(cal_output');
        disp('***************************************************');
    end
end