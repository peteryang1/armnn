//
// Copyright © 2017 Arm Ltd. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "NeonActivationWorkload.hpp"
#include <aclCommon/ArmComputeUtils.hpp>

namespace armnn
{

arm_compute::Status NeonActivationWorkloadValidate(const TensorInfo& input,
                                                   const TensorInfo& output,
                                                   const ActivationDescriptor& descriptor)
{
    const arm_compute::TensorInfo aclInput = armcomputetensorutils::BuildArmComputeTensorInfo(input);
    const arm_compute::TensorInfo aclOutput = armcomputetensorutils::BuildArmComputeTensorInfo(output);

    const arm_compute::ActivationLayerInfo activationLayerInfo =
        ConvertActivationDescriptorToAclActivationLayerInfo(descriptor);

    if (input.GetDataType() == DataType::QuantisedAsymm8 &&
        activationLayerInfo.activation() == arm_compute::ActivationLayerInfo::ActivationFunction::LOGISTIC)
    {
        return arm_compute::Status{arm_compute::ErrorCode::RUNTIME_ERROR,
                                   "Neon: Logistic Activations unsupported with QAsymm8 data type."};
    }

    return arm_compute::NEActivationLayer::validate(&aclInput,
                                                    &aclOutput,
                                                    activationLayerInfo);
}

NeonActivationWorkload::NeonActivationWorkload(const ActivationQueueDescriptor& descriptor,
                                               const WorkloadInfo& info)
    : BaseWorkload<ActivationQueueDescriptor>(descriptor, info)
{
    m_Data.ValidateInputsOutputs("NeonActivationWorkload", 1, 1);

    const arm_compute::ActivationLayerInfo activationLayerInfo =
        ConvertActivationDescriptorToAclActivationLayerInfo(m_Data.m_Parameters);

    arm_compute::ITensor& input = boost::polymorphic_downcast<INeonTensorHandle*>(m_Data.m_Inputs[0])->GetTensor();
    arm_compute::ITensor& output = boost::polymorphic_downcast<INeonTensorHandle*>(m_Data.m_Outputs[0])->GetTensor();

    m_ActivationLayer.configure(&input, &output, activationLayerInfo);
}

void NeonActivationWorkload::Execute() const
{
    ARMNN_SCOPED_PROFILING_EVENT_NEON("NeonActivationWorkload_Execute");
    m_ActivationLayer.run();
}

} //namespace armnn
