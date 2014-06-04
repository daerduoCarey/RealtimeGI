% train NN
function net = trainNNs(file)
    [in, out] = loadData(file);
    layers = [20, 10];
    net = train_NN(in, out, layers);
end
