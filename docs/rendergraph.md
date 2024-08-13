# Render Graph

## Introduction
RenderGraph是一个用于描述和管理渲染过程有向无环图。由寒霜引擎在2017年GDC引入（[GDC Vault - FrameGraph: Extensible Rendering Architecture in Frostbite](https://www.gdcvault.com/play/1024612/FrameGraph-Extensible-Rendering-Architecture-in)）：
![RenderGraph Overview](https://typora-yy.oss-cn-hangzhou.aliyuncs.com/Typora-img/image-20240813153513522.png)

最先为了解决：

- 高耦合
- 资源手动管理
- Debug困难
- 拓展困难
- 即时渲染模式

等问题

### RenderGraph系统的工作原理

1. **节点定义**: 每个渲染节点代表一个独立的渲染任务，例如几何体绘制、阴影计算或后处理效果。每个节点包含输入和输出资源，例如纹理、缓冲区等。

2. **依赖关系**: 节点之间的依赖关系定义了渲染任务的执行顺序。一个节点的输出可以作为另一个节点的输入，从而形成一个有向无环图（DAG）。

3. **资源管理**: RenderGraph系统负责管理渲染过程中使用的资源，包括创建、销毁和重用资源。通过分析节点之间的依赖关系，系统可以优化资源的使用，减少内存占用和带宽消耗。

4. **执行调度**: 在渲染过程中，RenderGraph系统根据节点的依赖关系调度渲染任务的执行。系统会确保所有依赖的输入资源在任务执行前已经准备好，从而保证渲染结果的正确性。



### RenderGraph系统分析

#### RenderGraph.h

`RenderGraph.h`文件包含了RenderGraph系统的核心实现。RenderGraph系统的主要功能包括：

1. **资源管理**: RenderGraph通过`createBuffer`、`createTexture`等方法来创建资源，并通过`importBuffer`、`importTexture`等方法导入外部资源。资源在内部以`RenderGraphHandle`的形式进行管理。
    - `RenderGraphHandle createBuffer(const std::string& name, const RenderGraphBuffer::Descriptor& desc)`: 创建一个新的缓冲区资源。
    - `RenderGraphHandle importBuffer(const std::string& name, Buffer* hwBuffer)`: 导入一个外部缓冲区资源。
    - `RenderGraphHandle createTexture(const std::string& name, const RenderGraphTexture::Descriptor& desc)`: 创建一个新的纹理资源。
    - `RenderGraphHandle importTexture(const std::string& name, SgImage* hwTexture, bool addRef)`: 导入一个外部纹理资源。
    
2. **节点管理**: RenderGraph通过`addGraphicPass`、`addComputePass`、`addRaytracingPass`等方法来添加不同类型的渲染节点。每个节点代表一个渲染任务，并包含输入和输出资源。
    - `void addGraphicPass(const std::string& name, const GraphicSetup& setup, GraphicsExecute&& execute)`: 添加一个图形渲染节点。
    - `void addComputePass(const std::string& name, const ComputeSetUp& setup, ComputeExecute&& execute)`: 添加一个计算渲染节点。
    - `void addRaytracingPass(const std::string& name, const RayTracingSetup& setup, RaytracingExecute&& execute)`: 添加一个光线追踪渲染节点。
    
3. **依赖关系管理**: RenderGraph通过`readTexture`、`writeTexture`等方法来定义节点之间的依赖关系。依赖关系决定了渲染任务的执行顺序。
    - `RenderGraph::Builder& readTexture(RenderGraphHandle input, RenderGraphTexture::Usage usage)`: 定义一个节点读取纹理资源的依赖关系。
    - `RenderGraph::Builder& writeTexture(RenderGraphHandle output, RenderGraphTexture::Usage usage)`: 定义一个节点写入纹理资源的依赖关系。
    - `RenderGraph::Builder& readBuffer(RenderGraphHandle input, RenderGraphBuffer::Usage usage)`: 定义一个节点读取缓冲区资源的依赖关系。
    - `RenderGraph::Builder& writeBuffer(RenderGraphHandle output, RenderGraphBuffer::Usage usage)`: 定义一个节点写入缓冲区资源的依赖关系。
    
4. **执行调度**: RenderGraph通过`compile`方法来分析和优化节点之间的依赖关系，并通过`execute`方法来调度渲染任务的执行。

    - `void compile()`: 分析和优化节点之间的依赖关系。
    - `void execute(CommandBuffer& commandBuffer)`: 调度渲染任务的执行。

#### RenderGraphNode.h

`RenderGraphNode.h`文件定义了RenderGraph系统中的节点基类`RenderGraphNode`。每个节点代表一个独立的渲染任务，包含以下特性：

1. **引用计数**: 每个节点包含一个引用计数`refCount`，用于管理节点的生命周期。

2. **名称管理**: 每个节点包含一个名称`mName`，用于标识节点。

3. **虚函数**: `RenderGraphNode`类包含一些虚函数，如`isResource`，用于判断节点是否是资源节点。

#### RenderGraphPass.h

`RenderGraphPass.h`文件定义了RenderGraph系统中的渲染节点类`RenderGraphPassBase`及其派生类。每个渲染节点代表一个具体的渲染任务，包含以下特性：

1. **执行函数**: 每个渲染节点包含一个执行函数`execute`，用于执行具体的渲染任务。

2. **数据管理**: 每个渲染节点包含一个数据成员`data`，用于存储渲染任务的相关数据。

3. **类型定义**: `RenderGraphPass.h`文件定义了不同类型的渲染节点，如`GraphicRenderGraphPass`、`ComputeRenderGraphPass`、`RaytracingRenderGraphPass`，分别用于图形渲染、计算任务和光线追踪任务。

### RenderGraph系统的理解

RenderGraph系统是一个用于描述和管理渲染过程的系统。它通过定义渲染节点和它们之间的依赖关系来组织渲染任务。每个渲染节点代表一个独立的渲染任务，包含输入和输出资源。节点之间的依赖关系定义了渲染任务的执行顺序。RenderGraph系统负责管理渲染过程中使用的资源，包括创建、销毁和重用资源。通过分析节点之间的依赖关系，系统可以优化资源的使用，减少内存占用和带宽消耗。在渲染过程中，RenderGraph系统根据节点的依赖关系调度渲染任务的执行，确保所有依赖的输入资源在任务执行前已经准备好，从而保证渲染结果的正确性。

### RenderGraph系统的流程

1. **定义节点**: 使用`addGraphicPass`、`addComputePass`、`addRaytracingPass`等方法定义渲染节点。

2. **定义依赖关系**: 使用`readTexture`、`writeTexture`等方法定义节点之间的依赖关系。

3. **资源管理**: 使用`createBuffer`、`createTexture`等方法创建资源，使用`importBuffer`、`importTexture`等方法导入外部资源。

4. **编译和优化**: 使用`compile`方法分析和优化节点之间的依赖关系。

5. **执行渲染任务**: 使用`execute`方法调度渲染任务的执行。

