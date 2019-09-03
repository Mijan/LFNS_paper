function [] = plotBE(log_file_name)
folder_index = strfind(log_file_name, '/');
file_name_index = strfind(log_file_name, 'log_file.txt');

folder_name = log_file_name(1:folder_index(end));
file_name = log_file_name(1: file_name_index -1);

posterior = dlmread([file_name, 'posterior.txt']);
log_weights = dlmread([file_name, 'posterior_log_weights.txt']);
weights = exp(log_weights);

logs = dlmread(log_file_name, '\t', 1, 0);
max_iteration_nbr = size(logs, 1);

log_zd = logs(:, 7);
log_zl = logs(:, 9);
log_ztot = logs(:, 5);

log_var_zd = logs(:, 8);
log_var_zl = logs(:, 10);
log_var_ztot = logs(:, 6);
log_var_min = logs(:, 11);
indices(:, 1) = 1: max_iteration_nbr;

% log_std_zd = 0.5*log_var_zd;
% max_log = max(log_zd, log_std_zd);
% zd_down = log(max(exp(log_zd-max_log) -exp(log_std_zd -max_log), 0)) +max_log;
% zd_up = log(exp(log_zd-max_log) + exp(log_std_zd-max_log)) +max_log;
% zd_down(isinf(zd_down)) = min(log_zd);
% fill([indices;flipud(indices)],[zd_down;flipud(zd_up)],[.93 .84 .84],'linestyle','none');
% hold on;
%
%
% log_std_zl = 0.5*log_var_zl;
% max_log = max(log_zl, log_std_zl);
% zl_down = log(max(exp(log_zl-max_log) -exp(log_std_zl -max_log), 0)) +max_log;
% zl_up = log(exp(log_zl-max_log) + exp(log_std_zl-max_log)) +max_log;
% zl_down(isinf(zl_down)) = min(log_zd);
% fill([indices(1:end);flipud(indices(1:end))],[zl_down;flipud(zl_up)],[.76 .87 .78],'linestyle','none');
%
%
% log_std_ztot = 0.5*log_var_ztot;
% max_log = max(log_ztot, log_std_ztot);
% zLFNS_down = log(max(exp(log_ztot-max_log) -exp(log_std_ztot -max_log), 0)) +max_log;
% zLFNS_up = log(exp(log_ztot-max_log) + exp(log_std_ztot-max_log)) +max_log;
% zLFNS_down(isinf(zLFNS_down)) = min(log_zd);
% fill([indices;flipud(indices)],[zLFNS_down;flipud(zLFNS_up)],[.73 .83 .96],'linestyle','none');
%
% plot(log_zd, 'Linewidth', 1);
% plot(log_zl, 'Linewidth', 1);
% plot(log_ztot, 'Linewidth', 1);


%% plot lin scale
figure();

lfns_error = logs(:, end-1);
max_error = logs(:, end-2);
% mean_error = logs(:, 9) - logs(:, 7);
times = logs(:, 4);
acceptance = logs(:, 3);
subplot(1, 4, 1);
plot(indices, max_error, 'Linewidth', 2);
hold on;
plot(indices, lfns_error, 'Linewidth', 2);
% plot(indices, mean_error, 'Linewidth', 2);
title('Error estimate');
legend({ '$\log(\Delta_{max}^m)$', '$\log(\Delta_{LFNS}^m)$'},'Interpreter', 'latex');
ylabel('$\log(\Delta^m)$', 'interpreter', 'latex');
xlabel('Iteration Nbr m', 'interpreter', 'latex');
grid on;
xlim([indices(1) indices(end)+1]);

subplot(1, 4, 2);
plot(indices, 0.5* log_var_ztot, 'Linewidth', 2, 'Color', [.73 .83 .96]);
hold on;
plot(indices, 0.5* log_var_min, 'Linewidth', 2, 'Color', [.31 .4 .58]);
title('Variance estimate');
legend({ '$\log(\hat{\sigma}_\textnormal{tot}^{2m})$', '$\log(\hat{\sigma}_\textnormal{min}^{2m})$'}, 'Interpreter', 'latex');
ylabel('$\log(\sigma^{2m})$', 'interpreter', 'latex');
xlabel('Iteration Nbr m', 'interpreter', 'latex');
grid on;
xlim([indices(1) indices(end)+1]);






subplot(1, 4, 3);
if exp(log_ztot(end)) > 0
    norm_const = 0;
else
    norm_const = log_ztot(end);
end
zd_norm =  exp(log_zd -norm_const);
log_var_zd_norm = log_var_zd - 2*norm_const;
std_zd_norm = exp(0.5*log_var_zd_norm);
zd_down = max(zd_norm -std_zd_norm, 0);
zd_up = zd_norm + std_zd_norm;
fill([indices;flipud(indices)],[zd_down;flipud(zd_up)],[.93 .84 .84],'linestyle','none');
hold on;

zl_norm = exp(log_zl -norm_const);
log_var_zl_norm = log_var_zl - 2*norm_const;
std_zl_norm = exp(0.5*log_var_zl_norm);
zl_down = max(zl_norm -std_zl_norm, 0);
zl_up =zl_norm+ std_zl_norm;
fill([indices(1:end);flipud(indices(1:end))],[zl_down;flipud(zl_up)],[.76 .87 .78],'linestyle','none');

ztot_norm = exp(log_ztot -norm_const);
log_var_ztot_norm = log_var_ztot - 2*norm_const;
std_ztot_norm = exp(0.5*(log_var_ztot_norm));
zLFNS_down = max(ztot_norm-std_ztot_norm, 0);
zLFNS_up = ztot_norm+ std_ztot_norm;
fill([indices;flipud(indices)],[zLFNS_down;flipud(zLFNS_up)],[.73 .83 .96],'linestyle','none');




log_var_min_norm = log_var_min - 2*norm_const;
std_std_min_norm = exp(0.5*(log_var_min_norm));
zmin_down = max(ztot_norm-std_std_min_norm, 0);
zmin_up = ztot_norm+ std_std_min_norm;
fill([indices;flipud(indices)],[zmin_down;flipud(zmin_up)],[.31 .4 .58],'linestyle','none');

plot(zd_norm, 'Linewidth', 2, 'Color', [.49 .18 .56] );
plot(zl_norm, 'Linewidth', 2,  'Color', [.47 .67 .19]);
plot(ztot_norm, 'Linewidth', 2, 'Color', [0.3 .75 .93]);


if(norm_const ~=0)
    max_val = ceil(max(zLFNS_up));
    int = (max_val / 10);
    yticks([0 :int :max_val])
    Z_tot = ['x10^{', num2str(log_ztot(end)*(log10(exp(1)))), '}'];
    
    ticklabels = {['0', Z_tot]};
    for(i = 1 : 10)
            ticklabels = [ticklabels, { [num2str(i* int)]}];
    end
    yticklabels(ticklabels);
end

g = gca;
grid on;
h = g.Children;
title('BE estimate');
legend([h(7), h(6), h(5), h(4), h(1), h(3), h(2)], {'$\pm\hat{\sigma}_\mathcal{D}^{m}$', '$\pm\hat{\sigma}_\mathcal{L}^{m}$', '$\pm\hat{\sigma}_\textnormal{tot}^{m}$', '$\pm\hat{\sigma}_\textnormal{min}^{m}$','$\hat{Z}_{\textnormal{tot}}$', '$\hat{Z}_{\mathcal{D}}$','$\hat{Z}_{\mathcal{L}}$'},'Interpreter', 'latex');
ylabel('$\hat{Z}$', 'interpreter', 'latex');
xlabel('Iteration Nbr m', 'interpreter', 'latex');
xlim([indices(1) indices(end)+1]);

subplot(1, 4, 4);
grid on;
yyaxis left;
plot(indices, acceptance, 'Linewidth', 2);
ylabel('Acceptance rate', 'interpreter', 'latex');
yyaxis right;

cum_times = cumsum(times);
if cum_times(end) > 2*3600
    plot(indices, cumsum(times/3600), 'Linewidth', 2);
    ylabel('Cumulative time in hours', 'interpreter', 'latex');
elseif cum_times(end) > 2*60
    plot(indices, cumsum(times/60), 'Linewidth', 2);
    ylabel('Cumulative time in minutes', 'interpreter', 'latex');
else
    plot(indices, cumsum(times), 'Linewidth', 2);
    ylabel('Cumulative time in seconds', 'interpreter', 'latex');
end


xlabel('Iteration Nbr m', 'interpreter', 'latex');
title('Computational effort');

end

