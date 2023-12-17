import numpy as np
import matplotlib.pyplot as plt

import data


# https://stackoverflow.com/questions/12523586/python-format-size-application-converting-b-to-kb-mb-gb-tb
def format_bytes(size):
    # 2**10 = 1024
    power = 2**10
    n = 0
    power_labels = {0 : '', 1: 'KiB', 2: 'MiB', 3: 'GiB', 4: 'TiB'}
    while size > power:
        size /= power
        n += 1

    return format(size, '.2f').rstrip('0').rstrip('.') + '' + power_labels[n]


def inter_vs_cluster_way_partitioning_vs_inter_intra(num_clusters_owned_by_client):
    # L1: 64KB, 4-way, 64-byte blocks
    d = data.inter_vs_cluster_way_partitioning_vs_inter_intra[num_clusters_owned_by_client]

    length = len(d['cache_slice_sizes'])

    # Can be controlled in orther
    first_idx = 1
    last_idx = length

    cache_sizes_numeric = d['cache_slice_sizes'][first_idx:last_idx]
    cache_sizes_readable = list(map(lambda x: format_bytes(x), cache_sizes_numeric))

    inter_misses = np.array(d['inter_node_misses'][first_idx:last_idx])
    way_partition_misses = np.array(d['way_partition_misses'][first_idx:last_idx])
    inter_intra_misses = np.array(d['inter_intra_misses'][first_idx:last_idx])

    inter_total_accesses = np.array(d['inter_node_accesses'][first_idx:last_idx]).sum(axis=1)
    way_partition_total_accesses = np.array(d['way_partition_accesses'][first_idx:last_idx]).sum(axis=1)
    inter_intra_total_accesses = np.array(d['inter_intra_accesses'][first_idx:last_idx]).sum(axis=1)

    # TODO: Miss rate (only for LLC, not counting L1) is super-high, always > 0.5. Is that normal?
    # TODO: Shall we plot the Miss rate or the absolute number of misses?
    inter_miss_rate = inter_misses / inter_total_accesses
    way_partition_miss_rate = way_partition_misses / way_partition_total_accesses
    inter_intra_miss_rate = inter_intra_misses / inter_intra_total_accesses

    def plot_miss_rate():
        # TODO: In some graphs, the Inter-Node partitioning line is hidden behind inter-intra node (in the ones
        #   that work properly). Fix this?

        plt.figure(figsize=(6.5, 4.5))

        plt.plot(cache_sizes_numeric, inter_miss_rate, marker='o', label='Inter Node Partitioning')
        plt.plot(cache_sizes_numeric, way_partition_miss_rate, marker='o', label='Way Partitioning')

        # TODO: Moreover, there seems to be some kind of bug for inter-intra node partitioning and the results
        #   don't make too much sense. Remove?
        plt.plot(cache_sizes_numeric, inter_intra_miss_rate, marker='o', label='Inter-Intra Node Partitioning')

        plt.title(f'Miss rate in GAP vs. Cache Size ({num_clusters_owned_by_client}/8 slices owned)')
        plt.xlabel('LLC Slice Size')
        plt.ylabel('Miss rate')
        plt.xticks(cache_sizes_numeric, cache_sizes_readable, rotation=45, ha='right')
        plt.legend()
        plt.grid(True)
        plt.tight_layout()
        plt.show()

    def plot_absolute_number_of_misses():
        # TODO: In some graphs, the Inter-Node partitioning line is hidden behind inter-intra node (in the ones
        #   that work properly). Fix this?

        # TODO: Maybe scale the y-axis so that the numbers are not so big?
        plt.figure(figsize=(6.5, 4.5))

        plt.plot(cache_sizes_numeric, inter_misses, marker='o', label='Inter Node Partitioning')
        plt.plot(cache_sizes_numeric, way_partition_misses, marker='o', label='Way Partitioning')

        # TODO: Moreover, there seems to be some kind of bug for inter-intra node partitioning and the results
        #   don't make too much sense. Remove?
        plt.plot(cache_sizes_numeric, inter_intra_misses, marker='o', label='Inter-Intra Node Partitioning')

        plt.title(f'LLC Misses in GAP vs. Cache Size ({num_clusters_owned_by_client}/8 slices owned)')
        plt.xlabel('LLC Slice Size')
        plt.ylabel('Number of LLC Misses')
        plt.xticks(cache_sizes_numeric, cache_sizes_readable, rotation=45, ha='right')
        plt.legend()
        plt.grid(True)
        plt.tight_layout()
        plt.show()

    plot_miss_rate()
    plot_absolute_number_of_misses()


def all_inter_vs_cluster_way_partitioning_vs_inter_intra():
    for key in data.inter_vs_cluster_way_partitioning_vs_inter_intra:
        inter_vs_cluster_way_partitioning_vs_inter_intra(key)

all_inter_vs_cluster_way_partitioning_vs_inter_intra()