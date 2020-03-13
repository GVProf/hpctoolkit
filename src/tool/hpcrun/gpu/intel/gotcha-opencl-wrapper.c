#include <gotcha/gotcha.h>
#include <stdio.h>
#include <CL/cl.h>
#include "opencl-gpu-api.h"

/* add this file's code to keren's library_map() code
	try to remember why static is being used*/ 

struct cl_callback {char* type; size_t size;};

// function declarations
typedef cl_command_queue (*clqueue_fptr)(cl_context, cl_device_id, cl_command_queue_properties, cl_int *);
typedef cl_int (*clkernel_fptr)(cl_command_queue, cl_kernel, cl_uint, const size_t*, const size_t*, const size_t*, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*clreadbuffer_fptr)(cl_command_queue, cl_mem, cl_bool, size_t, size_t, void *, cl_uint, const cl_event *, cl_event *);
typedef cl_int (*clwritebuffer_fptr)(cl_command_queue, cl_mem, cl_bool, size_t, size_t, const void *, cl_uint, const cl_event *, cl_event *);
typedef cl_int (*clSetKernelArg_fptr)(cl_kernel, cl_uint, size_t, const void *);

cl_event* eventNullCheck(cl_event*);
void event_callback(cl_event, cl_int, void *);
static cl_command_queue clCreateCommandQueue_wrapper(cl_context, cl_device_id, cl_command_queue_properties, cl_int *);
static cl_int clEnqueueReadBuffer_wrapper(cl_command_queue, cl_mem, cl_bool, size_t, size_t, void *, cl_uint, const cl_event *, cl_event *);
static cl_int clEnqueueWriteBuffer_wrapper(cl_command_queue, cl_mem, cl_bool, size_t, size_t, const void *, cl_uint, const cl_event *, cl_event *);
//static cl_int clSetKernelArg_wrapper(cl_kernel, cl_uint, size_t, const void *);
static void mem_intercept(cl_kernel, size_t, cl_event *);
static cl_int clEnqueueNDRangeKernel_wrapper(cl_command_queue, cl_kernel, cl_uint, const size_t*, const size_t*, const size_t*,																		   cl_uint, const cl_event*, cl_event*);

// global variables
static gotcha_wrappee_handle_t clCreateCommandQueue_handle;
static gotcha_wrappee_handle_t clEnqueueNDRangeKernel_handle;
static gotcha_wrappee_handle_t clEnqueueReadBuffer_handle;
static gotcha_wrappee_handle_t clEnqueueWriteBuffer_handle;
//static gotcha_wrappee_handle_t clSetKernelArg_handle;

static cl_command_queue clCreateCommandQueue_wrapper(cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_int *errcode_ret)
{
	properties |= (cl_command_queue_properties)CL_QUEUE_PROFILING_ENABLE; // enabling profiling
	clqueue_fptr clCreateCommandQueue_wrappee = (clqueue_fptr) gotcha_get_wrappee(clCreateCommandQueue_handle);
	return clCreateCommandQueue_wrappee(context, device, properties, errcode_ret);
}

static cl_int clEnqueueNDRangeKernel_wrapper(cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, const size_t *global_work_offset, 													   const size_t *global_work_size, const size_t *local_work_size, cl_uint num_events_in_wait_list, 															 const cl_event *event_wait_list, cl_event *event)
{
	event = eventNullCheck(event);
	//createNodeForKernel("test_id_k1");
    clkernel_fptr clEnqueueNDRangeKernel_wrappee = (clkernel_fptr) gotcha_get_wrappee(clEnqueueNDRangeKernel_handle);
	cl_int return_status = clEnqueueNDRangeKernel_wrappee(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event);
	
	// setup callback where kernel profiling info will be collected
    //kernel_cb.size = NULL;
	struct cl_callback *kernel_cb = (struct cl_callback*) malloc(sizeof(struct cl_callback));
	kernel_cb->type = "cl_kernel";
	printf("registering callback for type: %s\n", kernel_cb->type);
	clSetEventCallback(*event, CL_COMPLETE, &event_callback, kernel_cb);
	return return_status;
}

static cl_int clEnqueueReadBuffer_wrapper (cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, size_t offset, size_t cb,															 void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event)
{
	event = eventNullCheck(event);
	//cct_node* node = createNode();
    clreadbuffer_fptr clEnqueueReadBuffer_wrappee = (clreadbuffer_fptr) gotcha_get_wrappee(clEnqueueReadBuffer_handle);
	// cb variable contains the size of the data being read from device to host. 
	printf("%zu(bytes) of data being transferred from device to host\n", cb); // pass this data
	cl_int return_status = clEnqueueReadBuffer_wrappee(command_queue, buffer, blocking_read, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event);
	struct cl_callback *mem_transfer_cb = (struct cl_callback*) malloc(sizeof(struct cl_callback));
	mem_transfer_cb->type = "cl_mem_transfer";
	printf("registering callback for type: %s\n", mem_transfer_cb->type);
	clSetEventCallback(*event, CL_COMPLETE, &event_callback, mem_transfer_cb);
	return return_status;
}

static cl_int clEnqueueWriteBuffer_wrapper(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t cb,														  const void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event)
{
	event = eventNullCheck(event);
	//createNodeForMemTransfer("test_id_m_w1_1");
	mem_intercept(NULL, cb, event);
    clwritebuffer_fptr clEnqueueWriteBuffer_wrappee = (clwritebuffer_fptr) gotcha_get_wrappee(clEnqueueWriteBuffer_handle);
	cl_int return_status = clEnqueueWriteBuffer_wrappee(command_queue, buffer, blocking_write, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event);
	struct cl_callback *mem_transfer_cb = (struct cl_callback*) malloc(sizeof(struct cl_callback));
	mem_transfer_cb->type = "cl_mem_transfer";
	printf("cb_type: %s\n", mem_transfer_cb->type);
	clSetEventCallback(*event, CL_COMPLETE, &event_callback, mem_transfer_cb);
	return return_status;
}

/* this fn can be called to indirectly pass buffer to devices. If so, we cannot get timing information, since there are no events tied to this method
 * So, the memory transfer time could be a part of enqueueKernel (to verify)*/
/*deferred
cl_int clSetKernelArg_wrapper(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void *arg_value)
{
	createNodeForMemTransfer("test_id_m_w2_1");
	void *param_value;
	size_t mem_obj_size;

	if (!arg_value)
		printf("NOT NULL\n");
	else
		printf("NULL\n");
	cl_mem temp = (cl_mem) arg_value;
	printf("check3: %s\n", CL_MEM_SIZE);
    clGetMemObjectInfo((cl_mem)arg_value, CL_MEM_SIZE, sizeof(mem_obj_size), &mem_obj_size, NULL);
	printf("checking size using meminfo: %lu\n", mem_obj_size);
	mem_intercept(kernel, arg_size, NULL);
	printf("size of setargs: %zu\n", sizeof(*arg_value));
    clSetKernelArg_fptr clSetKernelArg_wrappee = (clSetKernelArg_fptr) gotcha_get_wrappee(clSetKernelArg_handle);
	cl_int return_status = clSetKernelArg_wrappee(kernel, arg_index, arg_size, arg_value);
	return return_status;
}
*/

void mem_intercept(cl_kernel kernel, size_t size, cl_event *event)
{
	printf("%zu(bytes) of data being transferred from host to device\n", size); // pass this data
	struct cl_callback *mem_transfer_cb = (struct cl_callback*) malloc(sizeof(struct cl_callback));
	// pass data
}

cl_event* eventNullCheck(cl_event* event)
{
	if(!event)
	{
    	cl_event* new_event = (cl_event*)malloc(sizeof(cl_event));
		return new_event;
	}
	return event;
}

void event_callback(cl_event event, cl_int event_command_exec_status, void *user_data)
{
	char* type = ((struct cl_callback*)user_data)->type;
	printf("inside callback function. The command execution status for the event is : %d. The callback type is: %s\n", event_command_exec_status, type);
	// extracting the profiling numbers
	//struct profilingData* pd = getKernelProfilingInfo(event); // pass this data
	cl_ulong commandQueued = 0;
	cl_ulong commandSubmit = 0;
	cl_ulong commandStart = 0;
	cl_ulong commandEnd = 0;
	cl_int errorCode = CL_SUCCESS;

	errorCode |= clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_QUEUED, sizeof(commandQueued), &commandQueued, NULL);
	errorCode |= clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_SUBMIT, sizeof(commandSubmit), &commandSubmit, NULL);
	errorCode |= clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(commandStart), &commandStart, NULL);
	errorCode |= clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(commandEnd), &commandEnd, NULL);
	if (errorCode != CL_SUCCESS)
		printf("error in collecting profiling data.\n");
	else
	{
		/*struct profilingData *pd = (struct profilingData*) malloc(sizeof(struct profilingData));
		pd->queueTime = commandQueued;
		pd->submitTime = commandSubmit;
		pd->startTime = commandStart;
		pd->endTime = commandEnd;*/
		/*if (type == "cl_kernel")
			updateNodeWithKernelProfileData("test_id", pd);
		else if (type == "cl_mem_transfer")
			updateNodeWithMemTransferProfileData("test_id", pd);*/
	}
}

static gotcha_binding_t queue_wrapper[] = {{"clCreateCommandQueue", (void*) clCreateCommandQueue_wrapper, &clCreateCommandQueue_handle}};
static gotcha_binding_t kernel_wrapper[] = {{"clEnqueueNDRangeKernel", (void*)clEnqueueNDRangeKernel_wrapper, &clEnqueueNDRangeKernel_handle}};
static gotcha_binding_t buffer_wrapper[] = {{"clEnqueueReadBuffer", (void*) clEnqueueReadBuffer_wrapper, &clEnqueueReadBuffer_handle},
											{"clEnqueueWriteBuffer", (void*) clEnqueueWriteBuffer_wrapper, &clEnqueueWriteBuffer_handle}};
//{"clSetKernelArg", (void*) clSetKernelArg_wrapper, &clSetKernelArg_handle}

__attribute__((constructor)) void construct()
{   
	gotcha_wrap(queue_wrapper, 1, "queue_intercept");
	gotcha_wrap(kernel_wrapper, 1, "kernel_intercept");
	gotcha_wrap(buffer_wrapper, 2, "memory_intercept");
}

/*
gcc gotcha-opencl-wrapper.c -lOpenCL -shared -lgotcha -fPIC -o gotcha-opencl-wrapper.so
LD_PRELOAD=./gotcha-opencl-wrapper.so ./hello
*/
