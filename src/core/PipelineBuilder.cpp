#include "PipelineBuilder.hpp"
#include "VulkanUtils.hpp"


PipelineBuilder::PipelineBuilder(VkDevice device, VkRenderPass renderPass)
: device(device), renderPass(renderPass), colorBlendAttachment{} {
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    }

PipelineBuilder& PipelineBuilder::setShaders(const std::string& vertPath, const std::string& fragPath) {
    auto vertCode = VulkanUtils::readFile(vertPath);
    auto fragCode = VulkanUtils::readFile(fragPath);
    
    VkShaderModule vertShader = VulkanUtils::createShaderModule(device, vertCode);
    VkShaderModule fragShader = VulkanUtils::createShaderModule(device, fragCode);
    
    shaderStages = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            vertShader,
            "main",
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            fragShader,
            "main",
            nullptr
        }
    };
    
    return *this;
}
PipelineBuilder& PipelineBuilder::setInputAssembly(VkPrimitiveTopology topology) {
    inputAssembly.topology = topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    return *this;
}

PipelineBuilder& PipelineBuilder::setViewport(VkExtent2D extent) {
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor.offset = {0, 0};
    scissor.extent = extent;

    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    return *this;
}
PipelineBuilder& PipelineBuilder::setColorBlending() { //Смешивание цветов
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    return *this;
}
PipelineBuilder& PipelineBuilder::enableDepthTest(bool enable) { //Сравнение глубины текущего фрагмента с буфером глубины
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = enable ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = enable ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    return *this;
}
PipelineBuilder& PipelineBuilder::enableBlending() { // Включает альфа-смешивание final_color = src.rgba * src.alpha + dst.rgba * (1 - src.alpha)
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    return *this;
}
PipelineBuilder& PipelineBuilder::setRasterizer(){
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // VK_POLYGON_MODE_LINE 
    rasterizer.lineWidth = 1.0f; // Ширина линий VK_POLYGON_MODE_LINE
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // Отрисовка в зависимости "видит" ли это камера
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // Как идут грани(фронтальные и не фронтальные)
    rasterizer.depthBiasEnable = VK_FALSE;
    return *this;
}
PipelineBuilder& PipelineBuilder::setMultisampling(){
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    return *this;
}

PipelineBuilder& PipelineBuilder::setCullMode(VkCullModeFlags cullMode) { //Границы
    rasterizer.cullMode = cullMode;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.lineWidth = 1.0f;
    return *this;
}
PipelineBuilder& PipelineBuilder::setDynamicState(std::vector<VkDynamicState> dynamicStates) {
    dynamicStates_ = std::move(dynamicStates); 
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates_.size());
    dynamicState.pDynamicStates = dynamicStates_.data();
    return *this;
}
PipelineBuilder& PipelineBuilder::setPolygonMode(VkPolygonMode mode) {
    rasterizer.polygonMode = mode;
    return *this;
}
PipelineBuilder& PipelineBuilder::setPipelineLayout(VkPipelineLayout layout) {
    pipelineLayout = layout;
    return *this;
}

PipelineBuilder& PipelineBuilder::setVertexInfo(){
    bindingDescription_ = Vertex::getBindingDescription();
    attributeDescriptions_ = Vertex::getAttributeDescriptions();

    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions_.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription_;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions_.data();

    return *this;
}
PipelineBuilder& PipelineBuilder::setDepth(){
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS; // придерживаемся правила, что чем меньше глубина, тем ближе, поэтому глубина новых фрагментов должна быть меньше.
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional
    return *this;
}



VkPipelinePtr PipelineBuilder::build() {
    
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly; // как выводим геометрию(палки или треугольники)
    pipelineInfo.pViewportState = &viewportState; //область отрисовки
    pipelineInfo.pRasterizationState = &rasterizer; //Внутри  геометрии создаем "пиксели"
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending; // Смешивание цветов (отключаем, используем только альфа-канал, он легче)
    pipelineInfo.pDynamicState = &dynamicState; //Динамические штуки в сцене
    pipelineInfo.layout = pipelineLayout; // Штука с push константами
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.subpass = 0;

    VkPipeline rawPipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &rawPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline!");
    }
    
    for (const auto& stage : shaderStages) {
        vkDestroyShaderModule(device, stage.module, nullptr);
    }
    
    return VkPipelinePtr(rawPipeline, VulkanDeleter<VkPipeline_T, vkDestroyPipeline, VkDevice>(device));
}