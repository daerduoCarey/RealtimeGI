function [in, out] = loadData(file)
    dat = load(file);
    in = dat(:, 1:13);
    out = dat(:, 14:16);
    in = in';
    out = out';
end