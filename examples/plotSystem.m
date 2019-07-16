function [] = plotSystem(summary_file_name)
%% The input is 
%   summary_file_name: The path and name of the model summary file. The
%                       path can be absolute or relative
%                       (./results/birth_death_model_summary.txt for 
%                       instance. The model summary file must be in the
%                       same folder as all other results files



folder_index = strfind(summary_file_name, '/');
file_name_index = strfind(summary_file_name, '_model_summary.txt');

if isempty(file_name_index)
    fprintf('The provided file musst be the *_model summary.txt file. It must be in the same folder as the other output files ( *_times.txt, _latent_states*.txt, *_measurements.txt)' );
    return;
end

folder_name = summary_file_name(1:folder_index(end));
file_name = [folder_name, summary_file_name(folder_index(end) +1 : file_name_index)];


 [param_names, species_names, scales, bounds, model_configurations] = readModelDescription(summary_file_name);
 
for model_configuration_nbr = model_configurations{:}
    latent_states_file = [file_name,  model_configuration_nbr, '_latent_states.txt'];
    measurement_states_file = [file_name,  model_configuration_nbr, '_measurements.txt'];
    times_file = [file_name, 'times.txt'];
    model_summary_file = summary_file_name;
    
    t = dlmread(times_file);
    latent_states = dlmread(latent_states_file);
    measurement = dlmread(measurement_states_file);
    
    
    
    num_states = length( species_names);
    num_simulations = size(latent_states, 1)/ num_states;
    
    
    num_cols = 1;
    if num_states > 1
        num_cols = 2;
    end
    num_rows = ceil(num_states / 2);
    
    
    if num_simulations == 1
        figure(1);
        plot(t, measurement, 'o');
        xlabel('time');
        ylabel('measurement');
        title(['Measurments ', model_configuration_nbr]);
    else
        figure(1);
        subplot(1, 2, 1);
        plot(t, measurement, 'o');
        xlabel('time');
        ylabel('measurement');
        title(['Measurments ',model_configuration_nbr]);
        
        subplot(1, 2, 2);
        plot(t, mean(measurement), 'o');
        xlabel('time');
        ylabel('measurement');
        title(['Mean measurments ', model_configuration_nbr]);
        
    end
    
    figure(2);
    for i = 1 : num_states
        subplot(num_rows, num_cols, i);
        plot(t, latent_states(i : num_states : end, :), 'LineWidth', 2);
        xlabel('time');
        ylabel('state');
        title([species_names{i}, model_configuration_nbr]);
    end
    
    if num_simulations > 1
        figure(3);
        for i = 1 : num_states
            subplot(num_rows, num_cols, i);
            plot(t, mean(latent_states(i : num_states : end, :)), 'LineWidth', 2);
            xlabel('time');
            ylabel('state');
            title(['mean ', species_names{i}, ' ', model_configuration_nbr]);
        end
    end
    
end
end