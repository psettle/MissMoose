# -*- coding: utf-8 -*-
"""
    pyeasyga module
    Modified by Andy Schneider to provide more useful functionality
"""

import random
import copy
from operator import attrgetter
from multiprocessing.pool import Pool
from functools import partial
import time

from six.moves import range

class Parallel_Fitness(object):
    ''' Pickle-able class for containing a fitness function. It has to be pickle-able
    so that it can be used by the multiprocessing libraries.
    '''
    def __init__(self, fitness_function):
        self.fitness_function = fitness_function
    def work(self, args):
        '''This function needs to be called with an array of 3 elements, containing
        individual, individual_id, seed_data in that order.
        '''
        if(self.fitness_function is not None):
            return self.fitness_function(args[0], args[1], args[2])

class GeneticAlgorithm(object):
    """Genetic Algorithm class.

    This is the main class that controls the functionality of the Genetic
    Algorithm.

    A simple example of usage:

    >>> # Select only two items from the list and maximise profit
    >>> from pyeasyga.pyeasyga import GeneticAlgorithm
    >>> input_data = [('pear', 50), ('apple', 35), ('banana', 40)]
    >>> easyga = GeneticAlgorithm(input_data)
    >>> def fitness (member, data):
    >>>     return sum([profit for (selected, (fruit, profit)) in
    >>>                 zip(member, data) if selected and
    >>>                 member.count(1) == 2])
    >>> easyga.fitness_function = fitness
    >>> easyga.run()
    >>> print easyga.best_individual()

    """

    def __init__(self,
                 seed_data,
                 population_size=50,
                 generations=100,
                 crossover_probability=0.8,
                 mutation_probability=0.2,
                 elitism=True,
                 maximise_fitness=True,
                 parallel_process=False):
        """Instantiate the Genetic Algorithm.

        :param seed_data: input data to the Genetic Algorithm
        :type seed_data: list of objects
        :param int population_size: size of population
        :param int generations: number of generations to evolve
        :param float crossover_probability: probability of crossover operation
        :param float mutation_probability: probability of mutation operation
        :param function generation_callback: function called when a generation completes

        """
        self.seed_data = seed_data
        self.population_size = population_size
        self.generations = generations
        self.crossover_probability = crossover_probability
        self.mutation_probability = mutation_probability
        self.elitism = elitism
        self.maximise_fitness = maximise_fitness
        self.generation_callback = None
        self.parallel_process = parallel_process

        self.current_generation = []

        def create_individual(seed_data):
            """Create a candidate solution representation.

            e.g. for a bit array representation:

            >>> return [random.randint(0, 1) for _ in range(len(data))]

            :param seed_data: input data to the Genetic Algorithm
            :type seed_data: list of objects
            :returns: candidate solution representation as a list

            """
            return [random.randint(0, 1) for _ in range(len(seed_data))]

        def crossover(parent_1, parent_2):
            """Crossover (mate) two parents to produce two children.

            :param parent_1: candidate solution representation (list)
            :param parent_2: candidate solution representation (list)
            :returns: tuple containing two children

            """
            index = random.randrange(1, len(parent_1))
            child_1 = parent_1[:index] + parent_2[index:]
            child_2 = parent_2[:index] + parent_1[index:]
            return child_1, child_2

        def mutate(individual):
            """Reverse the bit of a random index in an individual."""
            mutate_index = random.randrange(len(individual))
            individual[mutate_index] = (0, 1)[individual[mutate_index] == 0]

        def random_selection(population):
            """Select and return a random member of the population."""
            return random.choice(population)

        def tournament_selection(population):
            """Select a random number of individuals from the population and
            return the fittest member of them all.
            """
            if self.tournament_size == 0:
                self.tournament_size = 2
            members = random.sample(population, self.tournament_size)
            members.sort(
                key=attrgetter('fitness'), reverse=self.maximise_fitness)
            return members[0]

        self.fitness_function = None
        self.tournament_selection = tournament_selection
        self.tournament_size = self.population_size // 10
        self.random_selection = random_selection
        self.create_individual = create_individual
        self.crossover_function = crossover
        self.mutate_function = mutate
        self.selection_function = self.tournament_selection

        if(parallel_process):
            self.p = Pool(8)

    def create_initial_population(self):
        """Create members of the first population randomly.
        """
        initial_population = []
        for _ in range(self.population_size):
            genes = self.create_individual(self.seed_data)
            individual = Chromosome(genes)
            initial_population.append(individual)
        self.current_generation = initial_population

    def calculate_population_fitness(self):
        """Calculate the fitness of every member of the given population using
        the supplied fitness_function.
        """
        if(self.parallel_process):
            t = Parallel_Fitness(self.fitness_function)
            args = []
            for ind in range(len(self.current_generation)):
                args.append([])
                args[ind].append(self.current_generation[ind].genes)
                args[ind].append(ind)
                args[ind].append(self.seed_data)

            individual_fitnesses = self.p.map(t.work, args)

            for i in range(len(self.current_generation)):
                self.current_generation[i].fitness = individual_fitnesses[i]

        else:
            for individual in self.current_generation:
                individual.fitness = self.fitness_function(
                    individual.genes, self.current_generation.index(individual), self.seed_data)

    def rank_population(self):
        """Sort the population by fitness according to the order defined by
        maximise_fitness.
        """
        self.current_generation.sort(
            key=attrgetter('fitness'), reverse=self.maximise_fitness)

    def create_new_population(self):
        """Create a new population using the genetic operators (selection,
        crossover, and mutation) supplied.
        """
        new_population = []
        new_population_genes = []
        elite = copy.deepcopy(self.current_generation[0])
        selection = self.selection_function

        while len(new_population) < self.population_size:
            parent_1 = copy.deepcopy(selection(self.current_generation))
            parent_2 = copy.deepcopy(selection(self.current_generation))

            child_1, child_2 = parent_1, parent_2
            child_1.fitness, child_2.fitness = 0, 0

            if((child_1.genes in new_population_genes) or
               (child_2.genes in new_population_genes)):
                # If we've already seen either of these individuals, mutate them.
                can_mutate = True
                if(child_1.genes == child_2.genes):
                    # If they're the same, don't bother crossing them over.
                    can_crossover = False
                else:
                    can_crossover = random.random() < self.crossover_probability
            else:
                can_crossover = random.random() < self.crossover_probability
                can_mutate = random.random() < self.mutation_probability

            if can_crossover:
                child_1.genes, child_2.genes = self.crossover_function(
                    parent_1.genes, parent_2.genes)

            if can_mutate:
                self.mutate_function(child_1.genes)
                self.mutate_function(child_2.genes)

            new_population.append(child_1)
            new_population_genes.append(child_1.genes)
            if len(new_population) < self.population_size:
                new_population.append(child_2)
                new_population_genes.append(child_2.genes)

        if self.elitism:
            new_population[0] = elite

        self.current_generation = new_population

    def create_first_generation(self):
        """Create the first population, calculate the population's fitness and
        rank the population by fitness according to the order specified.
        """
        self.create_initial_population()
        self.calculate_population_fitness()
        self.rank_population()

    def create_next_generation(self):
        """Create subsequent populations, calculate the population fitness and
        rank the population by fitness in the order specified.
        """
        start = time.time()
        self.create_new_population()
        end = time.time()
        print("Generation made: {}".format(end - start))
        start = time.time()
        self.calculate_population_fitness()
        end = time.time()
        print("Generation processed: {}".format(end - start))
        start = time.time()
        self.rank_population()
        end = time.time()
        print("Generation ranked: {}".format(end - start))

    def run(self):
        self.generation_index = 0
        """Run (solve) the Genetic Algorithm."""
        self.create_first_generation()
        """Call the callback function with information if it exists."""
        if(self.generation_callback is not None):
            self.generation_callback(self.current_generation, self.generation_index)

        for _ in range(1, self.generations):
            self.generation_index += 1
            self.create_next_generation()
            if(self.generation_callback is not None):
                self.generation_callback(self.current_generation, self.generation_index)

    def best_individual(self):
        """Return the individual with the best fitness in the current
        generation.
        """
        best = self.current_generation[0]
        return (best.fitness, best.genes)

    def last_generation(self):
        """Return members of the last generation as a generator function."""
        return ((member.fitness, member.genes) for member
                in self.current_generation)


class Chromosome(object):
    """ Chromosome class that encapsulates an individual's fitness and solution
    representation.
    """
    def __init__(self, genes):
        """Initialise the Chromosome."""
        self.genes = copy.deepcopy(genes)
        self.fitness = 0

    def __repr__(self):
        """Return initialised Chromosome representation in human readable form.
        """
        return repr((self.fitness, self.genes))
