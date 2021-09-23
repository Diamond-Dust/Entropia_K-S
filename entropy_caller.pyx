cdef extern from "entropy.c":
    int read_all_from(char* address, int limit)

cpdef py_read_all_from(py_address, limit):
    py_byte_string = py_address.encode('UTF-8')
    cdef char* c_string = py_byte_string
    return read_all_from(c_string, limit)


cdef extern from "entropy.c":
    int increment(int a)

cpdef py_increment(a):
    return increment(a)




cdef extern from "entropy.c":
    int calculation(
        char* data_file_name,
        char* results_file_name,
        int compartment_size,
        int subcompartment_num,
        int result_placement
    )

cpdef py_calculation(
    data_file_name,
    results_file_name,
    compartment_size,
    subcompartment_num,
    result_placement
):

    data_file_name_byte_string = data_file_name.encode('UTF-8')
    cdef char* data_file_name_c_string = data_file_name_byte_string
    results_file_name_byte_string = results_file_name.encode('UTF-8')
    cdef char* results_file_name_c_string = results_file_name_byte_string
    return calculation(
        data_file_name_c_string,
        results_file_name_c_string,
        compartment_size,
        subcompartment_num,
        result_placement
    )



"""
from libc.stdio cimport FILE
from cpython cimport array


cdef extern from "entropy.c":
    int following_compartments(
        FILE*   output,
        double* compartments,
        int     compartment_size,
        int*    compartments_y,
        double* compartment_y_address,
        int     subcompartment_num,
        double* latest_data_address,
        double* min_y_address,
        double* max_y_address,
        int*    min_y_index_address,
        int*    max_y_index_address,
        int*    data_size_address,
        int*    input_size_address,
        double* result_address
    )

cpdef py_following_compartments(
    output,
    compartments,
    compartment_size,
    compartments_y,
    compartment_y_address,
    subcompartment_num,
    latest_data_address,
    min_y_address,
    max_y_address,
    min_y_index_address,
    max_y_index_address,
    data_size_address,
    input_size_address,
    result_address
):
    return following_compartments(
        output,
        compartments_arr,
        compartment_size,
        compartments_y,
        compartment_y_address,
        subcompartment_num,
        latest_data_address,
        min_y_address,
        max_y_address,
        min_y_index_address,
        max_y_index_address,
        data_size_address,
        input_size_address,
        result_address
    )
"""
