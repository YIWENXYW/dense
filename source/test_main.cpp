#include "arg_parse.hpp"
#include "analysis.hpp"
#include "color.hpp"
#include "csvr_sim.hpp"
#include "csvw_sim.hpp"
#include "datalogger.hpp"
#include "simulation_set.hpp"
#include "model_impl.hpp"
#include "context_determ.hpp"
#include <iostream>


int main(int argc, char *argv[])
{
    arg_parse::init(argc, argv);
    color::enable(!arg_parse::get<bool>("C", "no-color", 0, false));
    
    if (arg_parse::get<bool>("H", "help", false) || arg_parse::get<bool>("h", "usage", false) || argc == 1)
    {
        // # Display all possible command line arguments with descriptions
        cout << color::set(color::YELLOW) <<
            "[-H | -h | --help | --usage]    " << color::set(color::GREEN) <<
            "Print information about program's various command line arguments." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-C | --no-color]               " << color::set(color::GREEN) <<
            "Disable color in the terminal." << color::clear() << endl;
            
            
        // TODO Improve these two help dialogues
        cout << color::set(color::YELLOW) <<
            "[-G | --gradients]              " << color::set(color::GREEN) <<
            "Enable " << color::set(color::RED) <<
            "{TODO: WRITE DESCRIPTION}" << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-P | --perturb]                " << color::set(color::GREEN) <<
            "Enable " << color::set(color::RED) <<
            "{TODO: WRITE DESCRIPTION}" << color::clear() << endl;
            
            
        cout << color::set(color::YELLOW) <<
            "[-p | --param-list]    <string> " << color::set(color::GREEN) <<
            "Relative file location and name of the parameter list csv. \"../param_list.csv\", for example, excluding quotation marks." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-e | --data-export]   <string> " << color::set(color::GREEN) <<
            "Relative file location and name of the output of the data logger csv. \"../data_out.csv\", for example, excluding quotation marks." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-i | --data-import]   <string> " << color::set(color::GREEN) <<
            "Relative file location and name of csv data to import into the data logger. \"../data_in.csv\", for example, excluding quotation marks. Using this flag skips the simulation." << color::clear() << endl;
         cout << color::set(color::YELLOW) <<
            "[-o | --specie-option] <string> " << color::set(color::GREEN) <<
            "When -e is enabled, output only these species to file. When -i is enabled, this argument lets the simulation know that the import data file contains only these species. IF MORE THAN ONE SPECIE IS DESIRED, enclose the argument in quotation marks and seperate the species using commas (\',\'). For example, \"ph13, mh1, ph113\", including quotation marks. If only one specie is desired, no commas or quotation marks are necessary." << color::clear() << endl;
       cout << color::set(color::YELLOW) <<
            "[-c | --cell-total]       <int> " << color::set(color::GREEN) <<
            "Total number of cells to simulate." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-w | --total-width]      <int> " << color::set(color::GREEN) <<
            "Width of tissue to simulate. Height is inferred by c/w." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-s | --step-size]   <RATETYPE> " << color::set(color::GREEN) <<
            "Increment size in which the simulation progresses through time." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-a | --anlys-intvl] <RATETYPE> " << color::set(color::GREEN) <<
            "Analysis AND file writing interval. How frequently (in units of simulated seconds) data is fetched from simulation for analysis and/or file writing." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-r | --local-range] <RATETYPE> " << color::set(color::GREEN) <<
            "Range in which oscillation features are searched for. USING THIS ARGUMENT IMPLICITLY TOGGLES WHETHER ANALYSIS WILL RUN OR NOT." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-t | --time]             <int> " << color::set(color::GREEN) <<
            "Amount of time to simulate." << color::clear() << endl;
    }
    else
    {
        // Nothing can run without cell_total; make sure user has set it
        int cell_total;
        RATETYPE anlys_intvl;
        if (arg_parse::get<int>("c", "cell-total", &cell_total, true) && arg_parse::get<RATETYPE>("a", "anlys-intvl", &anlys_intvl, true))
        {
            // Allow for analysis to be optional
            RATETYPE local_range;
            const bool doAnlys = arg_parse::get<RATETYPE>("r", "local-range", &local_range, false);

            // If there's no -o argument, specie_option will default to all species
            specie_vec specie_option;
            arg_parse::get<specie_vec>("o", "specie-option", &specie_option, false);
            

            // Figure out whether we're importing or exporting
            string data_import;
            if (arg_parse::get<string>("i", "data-import", &data_import, false))
            {
                csvr_sim csvrs(data_import, cell_total, specie_option);
                
                // A good csvw debugging test
                //csvw_sim csvws("data_in_copy_out.csv", specie_option, &csvrs);

                // Analysis optional
                if (doAnlys)
                {
                    // Prepare analyses
                    //BasicAnalysis ba(&csvrs);
                    OscillationAnalysis *oa[specie_option.size()];
                    for (unsigned int i=0; i<specie_option.size(); i++)
                    {
                        oa[i] = new OscillationAnalysis(&csvrs, anlys_intvl, local_range, specie_option.at(i));
                    }

                    // Emulate a simulation
                    csvrs.run();
                    
                    // Print analyses
                    cout << endl << endl;
                    //ba.test();
                    for (unsigned int i=0; i<specie_option.size(); i++)
                    {
                        cout << specie_str[specie_option.at(i)] << endl;
                        oa[i]->test();
                        delete oa[i];
                    }
                }
                else
                {
                    // Warn user about kind of useless case
                    cout << color::set(color::YELLOW) << "Warning: Your current set of command line arguments produces a somewhat useless state. (No outputs are being generated.) Did you mean to include the \'-r | --local-range\' flag?" << color::clear() << endl;
                    
                    // No analysis, simply emulate a simulation
                    // This particular case is pointless at the moment, but having analysis be optional is useful under the data_export case.
                    csvr_sim csvrs(data_import, cell_total, specie_option);
                    csvrs.run();
                }
            }
            else // If not importing data
            {
                string param_list;
                int total_width;
                RATETYPE step_size, time;
                
                if ( arg_parse::get<string>("p", "param-list", &param_list, true) &&
                        arg_parse::get<int>("w", "total-width", &total_width, true) &&
                        arg_parse::get<RATETYPE>("s", "step-size", &step_size, true) &&
                        arg_parse::get<RATETYPE>("t", "time", &time, true) )
                {
                    simulation_set<simulation_determ> sim_set(
                        arg_parse::get<bool>("G", "gradients", false),
                        arg_parse::get<bool>("P", "perturb", false),
                        param_list, cell_total, total_width,
                        step_size, anlys_intvl, time);
                    
                    // Prepare data output
                    bool doCSVWS;
                    csvw_sim *csvws[sim_set.getSetCount()];
                    string data_export;
                    // Not a typo
                    if (doCSVWS = arg_parse::get<string>("e", "data-export", &data_export, false))
                    {
                        for (unsigned int i=0; i<sim_set.getSetCount(); i++)
                        {
                            // Set file name to "x_####.y"
                            string data_num = to_string(i);
                            data_num.insert(data_num.begin(), 4-data_num.size(), '0');
                            string data_mod = data_export.substr(0, data_export.find_last_of(".")) + "_" + data_num + data_export.substr(data_export.find_last_of("."));
                            cout << data_mod << endl;
                            csvws[i] = new csvw_sim(data_mod, anlys_intvl, cell_total, specie_option, &sim_set._sim_set[i]);
                        }
                    }
                    

                    // Analysis optional
                    if (doAnlys)
                    {
                        // Prepare analyses
                        //BasicAnalysis ba(&sim_set._sim_set[0]);
                        OscillationAnalysis *oa[sim_set.getSetCount()][specie_option.size()];
                        for (unsigned int i=0; i<sim_set.getSetCount(); i++)
                        {
                            for (unsigned int j=0; j<specie_option.size(); j++)
                            {
                                oa[i][j] = new OscillationAnalysis(&sim_set._sim_set[i], anlys_intvl, local_range, specie_option.at(j));
                            }
                        }

                        // Run the show
                        sim_set.simulate_sets();

                        // Print analyses
                        cout << endl << endl;
                        //ba.test();
                        for (unsigned int i=0; i<sim_set.getSetCount(); i++)
                        {
                            for (unsigned int j=0; j<specie_option.size(); j++)
                            {
                                cout << "set " << i << " for specie " << specie_str[specie_option.at(j)] << endl;
                                oa[i][j]->test();
                                delete oa[i][j];
                            }
                        }
                    }
                    else
                    {
                        // Warn user about kind of useless case
                        if (!doAnlys && !doCSVWS)
                        {
                            cout << color::set(color::YELLOW) << "Warning: Your current set of command line arguments produces a somewhat useless state. (No outputs are being generated.) Did you mean to use the \'-r | --local-range\' and/or \'-e | --data-export\' flag(s)?" << color::clear() << endl;
                        }

                        // No analysis, simply run the simulation and output it to file
                        sim_set.simulate_sets();
                    }

                    // Memory clean-up for CSV Writer
                    for (unsigned int i=0; i<sim_set.getSetCount() && doCSVWS; i++)
                    {
                        delete csvws[i];
                    }
                }
            }
        }
    }
}
