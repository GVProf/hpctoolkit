#include "roctracer-activity-translate.h"

// HSA_OP_ID_COPY is defined in hcc/include/hc_prof_runtime.h.
// However, this file will include many C++ code, making it impossible
// to compile with pure C.
// HSA_OP_ID_COPY is a constant with value 1 at the moment. 
#define HSA_OP_ID_COPY  1

static void
convert_kernel_launch
(
    gpu_activity_t *ga,
    roctracer_record_t *activity
)
{
    ga->kind = GPU_ACTIVITY_KIND_KERNEL;
    set_gpu_activity_interval(&ga->details.interval, activity->begin_ns, activity->end_ns);
    ga->details.kernel.correlation_id = activity->correlation_id;
}

static void
convert_memcpy
(
    gpu_activity_t *ga,
    roctracer_record_t *activity
)
{
    ga->kind = GPU_ACTIVITY_KIND_MEMCPY;
    set_gpu_activity_interval(&ga->details.interval, activity->begin_ns, activity->end_ns);
    ga->details.memcpy.correlation_id = activity->correlation_id;
}


static void
convert_memset
(
    gpu_activity_t *ga,
    roctracer_record_t *activity
)
{
    ga->kind = GPU_ACTIVITY_KIND_MEMSET;
    set_gpu_activity_interval(&ga->details.interval, activity->begin_ns, activity->end_ns);
    ga->details.memset.correlation_id = activity->correlation_id;
}

static void
convert_sync
(
    gpu_activity_t *ga,
    roctracer_record_t *activity,
    gpu_synchronization_type_t syncKind
)
{
    ga->kind = GPU_ACTIVITY_KIND_SYNCHRONIZATION;
    set_gpu_activity_interval(&ga->details.interval, activity->begin_ns, activity->end_ns);
    ga->details.synchronization.syncKind = syncKind;
    ga->details.synchronization.correlation_id = activity->correlation_id;
}



void
roctracer_activity_translate
(
 gpu_activity_t *ga,
 roctracer_record_t *record,
 cct_node_t *cct_node        
)
{
    const char * name = roctracer_op_string(record->domain, record->op, record->kind);
    ga->cct_node = cct_node;    

    if (record->domain == ACTIVITY_DOMAIN_HIP_API) {
        switch(record->op){
            case HIP_API_ID_hipMemcpy:
            case HIP_API_ID_hipMemcpyToSymbolAsync:
            case HIP_API_ID_hipMemcpyFromSymbolAsync:
            case HIP_API_ID_hipMemcpyDtoD:
            case HIP_API_ID_hipMemcpy2DToArray:
            case HIP_API_ID_hipMemcpyAsync:
            case HIP_API_ID_hipMemcpyFromSymbol:
            case HIP_API_ID_hipMemcpy3D:
            case HIP_API_ID_hipMemcpyAtoH:
            case HIP_API_ID_hipMemcpyHtoD:
            case HIP_API_ID_hipMemcpyHtoA:
            case HIP_API_ID_hipMemcpy2D:
            case HIP_API_ID_hipMemcpyPeerAsync:
            case HIP_API_ID_hipMemcpyDtoH:
            case HIP_API_ID_hipMemcpyHtoDAsync:
            case HIP_API_ID_hipMemcpyFromArray:
            case HIP_API_ID_hipMemcpy2DAsync:
            case HIP_API_ID_hipMemcpyToArray:
            case HIP_API_ID_hipMemcpyToSymbol:
            case HIP_API_ID_hipMemcpyPeer:
            case HIP_API_ID_hipMemcpyDtoDAsync:
            case HIP_API_ID_hipMemcpyDtoHAsync:
            case HIP_API_ID_hipMemcpyParam2D:
                convert_memcpy(ga, record);
                break;
            case HIP_API_ID_hipModuleLaunchKernel:
            case HIP_API_ID_hipLaunchCooperativeKernel:
            case HIP_API_ID_hipHccModuleLaunchKernel:
            case HIP_API_ID_hipExtModuleLaunchKernel:
                convert_kernel_launch(ga, record);
                break;
            case HIP_API_ID_hipCtxSynchronize:
                convert_sync(ga, record, GPU_SYNCHRONIZATION_UNKNOWN);
                break;            
            case HIP_API_ID_hipStreamSynchronize:
                convert_sync(ga, record, GPU_SYNCHRONIZATION_STREAM_SYNC);
                break;            
            case HIP_API_ID_hipStreamWaitEvent:
                convert_sync(ga, record, GPU_SYNCHRONIZATION_STREAM_EVENT_WAIT);
                break;            
            case HIP_API_ID_hipDeviceSynchronize:
                convert_sync(ga, record, GPU_SYNCHRONIZATION_UNKNOWN);
                break;            
            case HIP_API_ID_hipEventSynchronize:
                convert_sync(ga, record, GPU_SYNCHRONIZATION_EVENT_SYNC);
                break;
            case HIP_API_ID_hipMemset3DAsync:
            case HIP_API_ID_hipMemset2D:
            case HIP_API_ID_hipMemset2DAsync:
            case HIP_API_ID_hipMemset:
            case HIP_API_ID_hipMemsetD8:
            case HIP_API_ID_hipMemset3D:
            case HIP_API_ID_hipMemsetAsync:
            case HIP_API_ID_hipMemsetD32Async:
                convert_memset(ga, record);
                break;

        }
    } else if (record->domain == ACTIVITY_DOMAIN_HCC_OPS) {
        if (record->op == HSA_OP_ID_COPY){
            convert_memcpy(ga, record);
        }
    }
}