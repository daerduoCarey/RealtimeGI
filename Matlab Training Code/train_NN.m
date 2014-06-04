function net = train_NN(in, out, layers)
    net = feedforwardnet(layers, 'trainlm');
    net = train(net, in, out);
end
