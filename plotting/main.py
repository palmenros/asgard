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

# def plot_absolute_number_of_misses():
#     plt.figure(figsize=(6.5, 4.5))
#
#     # plt.plot(cache_sizes_numeric, inter_misses, marker='o', label='Inter Node Partitioning')
#     plt.plot(cache_sizes_numeric, way_partition_misses, marker='o', label='Way Partitioning')
#
#     plt.plot(cache_sizes_numeric, inter_intra_misses, marker='o', label='Inter-Intra Node Partitioning')
#
#     plt.title(f'LLC Misses in GAP PageRank vs. LLC Slice Size (Owning {num_clusters_owned_by_client} out of 8 slices)')
#     plt.xlabel('LLC Slice Size')
#     plt.ylabel('Number of LLC Misses')
#     plt.xticks(cache_sizes_numeric, cache_sizes_readable, rotation=45, ha='right')
#     plt.legend()
#     plt.grid(True)
#     plt.tight_layout()
#     plt.show()

def inter_vs_cluster_way_partitioning_vs_inter_intra(num_clusters_owned_by_client):
    # L1: 64KB, 4-way, 64-byte blocks
    d = data.inter_vs_cluster_way_partitioning_vs_inter_intra[num_clusters_owned_by_client]

    length = len(d['cache_slice_sizes'])

    # Can be controlled in orther
    first_idx = 1
    last_idx = length

    cache_sizes_numeric = d['cache_slice_sizes'][first_idx:last_idx]
    cache_sizes_readable = list(map(lambda x: format_bytes(x), cache_sizes_numeric))

    # inter_misses = np.array(d['inter_node_misses'][first_idx:last_idx])
    way_partition_misses = np.array(d['way_partition_misses'][first_idx:last_idx])
    inter_intra_misses = np.array(d['inter_intra_misses'][first_idx:last_idx])

    # inter_total_accesses = np.array(d['inter_node_accesses'][first_idx:last_idx]).sum(axis=1)
    way_partition_total_accesses = np.array(d['way_partition_accesses'][first_idx:last_idx]).sum(axis=1)
    inter_intra_total_accesses = np.array(d['inter_intra_accesses'][first_idx:last_idx]).sum(axis=1)

    # inter_miss_rate = inter_misses / inter_total_accesses
    way_partition_miss_rate = way_partition_misses / way_partition_total_accesses
    inter_intra_miss_rate = inter_intra_misses / inter_intra_total_accesses

    def plot_miss_rate():

        plt.figure(figsize=(6.5, 4.5))

        # plt.plot(cache_sizes_numeric, inter_miss_rate, marker='o', label='Inter Node Partitioning')
        plt.plot(cache_sizes_numeric, way_partition_miss_rate, marker='o', label='Way Partitioning')
        plt.plot(cache_sizes_numeric, inter_intra_miss_rate, marker='o', label='Inter-Intra Node Partitioning')

        plt.title(f'LLC miss rate in GAP PageRank')

        plt.xlabel('LLC Slice Size')
        plt.ylabel('Miss Rate')
        plt.xticks(cache_sizes_numeric, cache_sizes_readable, rotation=45, ha='right')
        plt.legend(prop={'size': 11})
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(f'img/miss_rate_{num_clusters_owned_by_client}_clusters.svg', format='svg', dpi=1200)
        plt.show()


    plot_miss_rate()

def intra_vs_way_partitioning():
    # L1: 64KB, 4-way, 64-byte blocks
    d = data.intra_vs_way_partitioning

    length = len(d['cache_slice_sizes'])

    # Can be controlled to change the data window
    first_idx = 1
    last_idx = length

    cache_sizes_numeric = d['cache_slice_sizes'][first_idx:last_idx]
    cache_sizes_readable = list(map(lambda x: format_bytes(x), cache_sizes_numeric))

    way_partition_misses = np.array(d['way_partition_misses'][first_idx:last_idx])
    intra_misses = np.array(d['intra_node_misses'][first_idx:last_idx])

    way_partition_total_accesses = np.array(d['way_partition_accesses'][first_idx:last_idx])
    intra_total_accesses = np.array(d['intra_node_accesses'][first_idx:last_idx])

    way_partition_miss_rate = way_partition_misses / way_partition_total_accesses
    intra_miss_rate = intra_misses / intra_total_accesses

    def plot_miss_rate():
        plt.figure(figsize=(6.5, 4.5))

        # plt.plot(cache_sizes_numeric, inter_miss_rate, marker='o', label='Inter Node Partitioning')
        plt.plot(cache_sizes_numeric, way_partition_miss_rate, marker='o', label='Way Partitioning')

        plt.plot(cache_sizes_numeric, intra_miss_rate, marker='o', label='Intra Node Partitioning')

        plt.title(f'LLC miss rate in GAP PageRank')

        plt.xlabel('LLC Size')
        plt.ylabel('Miss Rate')
        plt.xticks(cache_sizes_numeric, cache_sizes_readable, rotation=45, ha='right')
        plt.legend(prop={'size': 11})
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(f'img/intra_vs_way_partitioning.svg', format='svg', dpi=1200)
        plt.show()

    plot_miss_rate()


def all_inter_vs_cluster_way_partitioning_vs_inter_intra():
    for key in data.inter_vs_cluster_way_partitioning_vs_inter_intra:
        inter_vs_cluster_way_partitioning_vs_inter_intra(key)


# inter_vs_cluster_way_partitioning_vs_inter_intra(1)
intra_vs_way_partitioning()
