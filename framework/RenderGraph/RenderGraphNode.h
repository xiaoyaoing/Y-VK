class RenderGraphNode
{
public:
    uint32_t refCount{0};

    virtual ~RenderGraphNode() = default;
};
