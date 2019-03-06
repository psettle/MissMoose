#! Python 3

import random
from pyeasyga_mod import *
from array import array
import subprocess
import os
import traceback, sys

root_dir = os.path.dirname(os.path.realpath(__file__))
testframework_path = os.path.join(root_dir, "..\\x64\\Release\\TestFramework.exe")

# setup seed data
data = [{'name': 'activity_variable_min',             'value': 1.0,     'min': 1.0, 'max': 10.0},
        {'name': 'activity_variable_max',             'value': 12.0,    'min': 1.0, 'max': 300.0},
        {'name': 'common_sensor_weight_factor',       'value': 1.0,     'min': 1.0, 'max': 10.0},
        {'name': 'base_sensor_weight_factor_pir',     'value': 3.0,     'min': 1.0, 'max': 10.0},
        {'name': 'base_sensor_weight_factor_lidar',   'value': 3.5,     'min': 1.0, 'max': 10.0},
        {'name': 'road_proximity_factor_0',           'value': 1.4,     'min': 1.0, 'max': 10.0},
        {'name': 'road_proximity_factor_1',           'value': 1.2,     'min': 1.0, 'max': 10.0},
        {'name': 'road_proximity_factor_2',           'value': 1.0,     'min': 1.0, 'max': 10.0},
        {'name': 'common_sensor_trickle_factor',      'value': 1.0,     'min': 1.0, 'max': 10.0},
        {'name': 'base_sensor_trickle_factor_pir',    'value': 1.003,   'min': 1.0, 'max': 10.0},
        {'name': 'base_sensor_trickle_factor_lidar',  'value': 1.0035,  'min': 1.0, 'max': 10.0},
        {'name': 'road_trickle_proximity_factor_0',   'value': 1.004,   'min': 1.0, 'max': 10.0},
        {'name': 'road_trickle_proximity_factor_1',   'value': 1.002,   'min': 1.0, 'max': 10.0},
        {'name': 'road_trickle_proximity_factor_2',   'value': 1.0,     'min': 1.0, 'max': 10.0},
        {'name': 'activity_variable_decay_factor',    'value': 0.99,    'min': 0.8, 'max': 0.9999999999999},
        {'name': 'activity_decay_period_ms',          'value': 3.0,     'min': 1.0, 'max': 50.0},
        {'name': 'possible_detection_threshold_rs',   'value': 3.0,     'min': 1.0, 'max': 50.0},
        {'name': 'possible_detection_threshold_nrs',  'value': 4.0,     'min': 1.0, 'max': 50.0},
        {'name': 'detection_threshold_rs',            'value': 6.0,     'min': 1.0, 'max': 50.0},
        {'name': 'detection_threshold_nrs',           'value': 7.0,     'min': 1.0, 'max': 50.0},
        {'name': 'minimum_concern_signal_duration_s', 'value': 30.0,    'min': 10.0, 'max': 100.0},
        {'name': 'minimum_alarm_signal_duration_s',   'value': 60.0,    'min': 10.0, 'max': 100.0}]

class Genetic_Algo:
    '''
    This is a class for holding all the functions and bits of data
    relevant to running the genetic algorithm!
    '''

    def __init__(self):
        random.seed()
        # initialise the GA
        self.ga = GeneticAlgorithm(data,
                            population_size=500,
                            generations=30,
                            crossover_probability=0.8,
                            mutation_probability=0.2,
                            elitism=True,
                            maximise_fitness=True,
                            parallel_process=False)

        # print the GA's best solution; a solution is valid only if there are no collisions
        self.ga.generation_callback = self.generation_callback
        self.ga.fitness_function = self.fitness       # set the GA's fitness function
        self.ga.mutate_function = self.mutate
        self.ga.create_individual = self.create_individual
        self.ga.crossover_function = self.crossover
        self.ga.run()

        print(self.ga.best_individual()[0])
        print(self.ga.best_individual()[1])



    # define and set function to create a candidate solution representation
    def create_individual(self, data):
        individual = data[:]
        for item in individual:
            item['value'] = random.random() * (item['max'] - item['min']) + item['min']
        return individual

    # define and set the GA's crossover operation
    def crossover(self, parent_1, parent_2):
        ''' Make 2 children. coin toss every one of their values to see if it comes from parent 1 or 2.
        '''
        child_1 = copy.deepcopy(parent_1)
        child_2 = copy.deepcopy(parent_2)
        for idx in range(len(child_1)):
            child_1[idx]['value'] = child_1[idx]['value'] if random.random() > 0.5 else parent_2[idx]['value']
            child_2[idx]['value'] = child_2[idx]['value'] if random.random() > 0.5 else parent_1[idx]['value']

        return child_1, child_2

    def mutate_cross(self, parent_1, parent_2):
        child_1 = copy.deepcopy(parent_1)
        child_2 = copy.deepcopy(parent_2)
        for idx in range(len(child_1)):
            r = random.random()
            child_1[idx]['value'] = child_1[idx]['value'] * r + (1 - r) * parent_2[idx]['value']
            r = random.random()
            child_2[idx]['value'] = child_1[idx]['value'] * r + (1 - r) * parent_2[idx]['value']

        return child_1, child_2

    # define and set the GA's mutation operation
    def mutate(self, individual):
        """ 50% chance of changing each value in an individual by up to +/- 20%
        """
        for item in individual:
            if (random.random() > 0.5):
                # Change by a random value
                item['value'] = (random.random() - 0.5) * 0.4 * item['value']
                # Constrain by the min and max
                item['value'] = item['min'] if item['value'] < item['min'] else item['value']
                item['value'] = item['max'] if item['value'] > item['max'] else item['value']

    def generation_callback(self, current_generation, generation_index):
        """ Callback to handle information that's generated after an individual
        is processed.
        """
        print( 'best individual score gen {0}: '.format(generation_index) + str(self.ga.best_individual()[0]) )
        # Write the score and individual to a log file
        logfile_path = os.path.join(root_dir, 'output', 'logfile.txt')
        if os.path.exists(logfile_path):
            append_write = 'a' # append if already exists
        else:
            append_write = 'w' # make a new file if not
        with open(logfile_path, append_write) as logfile:
            logfile.write("gen {0} score: {1} values: {2} ".format(generation_index, self.ga.best_individual()[0], self.ga.best_individual()[1]) + '\n')

    # define a fitness function
    def fitness(self, individual, individual_id, data):
        # Create a binary file with the individual's values
        individual_bin_path = os.path.join(root_dir, 'output', 'individual_file{}.bin')
        output_file = open(individual_bin_path.format(individual_id), 'wb')
        float_array = self.individual_to_float_array(individual)
        float_array.tofile(output_file)
        output_file.close()
        FNULL = open(os.devnull, 'w')
        output = subprocess.call([testframework_path, '{}'.format(individual_id)], stdout=FNULL, stderr=subprocess.STDOUT)
        if(output != 0):
            print('Error in processing individual {}.'.format(individual_id))
            return 0
        else:
            # Try and read the output binary file that would be written by the test framework.
            try:
                data = array('f')
                score_bin_path = os.path.join(root_dir, 'output', 'individual_score{}.bin')
                score_file = open(score_bin_path.format(individual_id), 'rb')
                data.fromfile(score_file, 1)
                #print('Score for individual {}: {}'.format(individual_id, data[0]))
                return data[0]
            except Exception:
                print('Error in processing individual {}.'.format(individual_id))
                return 0


    def individual_to_float_array(self, individual):
        arr = []
        for item in individual:
            arr.append(item['value'])
        return array('f', arr)


if __name__ == '__main__':
    try:
        application = Genetic_Algo()
    except Exception:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        # print traceback
        traceback.print_tb(exc_traceback, limit=1, file=sys.stdout)
        # print exception
        traceback.print_exception(exc_type, exc_value, exc_traceback,
                                limit=2, file=sys.stdout)