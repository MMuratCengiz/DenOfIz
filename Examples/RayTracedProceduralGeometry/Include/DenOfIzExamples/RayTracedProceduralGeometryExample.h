#pragma once

#include <DenOfIzExamples/IExample.h>
#include <DenOfIzExamples/RayTracingHlslCompat.h>
#include <DenOfIzGraphics/Data/BatchResourceCopy.h>
#include <DenOfIzGraphics/Renderer/Common/CommandListRing.h>
#include <DenOfIzGraphics/Renderer/Graph/RenderGraph.h>
#include <DenOfIzGraphics/Utilities/Time.h>

namespace DenOfIz
{
    namespace AnalyticPrimitive
    {
        enum Enum
        {
            AABB = 0,
            Spheres,
            Count
        };
    } // namespace AnalyticPrimitive

    namespace VolumetricPrimitive
    {
        enum Enum
        {
            MetaBalls = 0,
            Count
        };
    } // namespace VolumetricPrimitive

    namespace SignedDistancePrimitive
    {
        enum Enum
        {
            MiniSpheres = 0,
            IntersectedRoundCube,
            SquareTorus,
            TwistedTorus,
            Cog,
            Cylinder,
            FractalPyramid,
            Count
        };
    } // namespace SignedDistancePrimitive

    namespace IntersectionShaderType
    {
        enum Enum
        {
            AnalyticPrimitive = 0,
            VolumetricPrimitive,
            SignedDistancePrimitive,
            Count
        };

        static uint32_t PerPrimitiveTypeCount( const Enum type )
        {
            switch ( type )
            {
            case AnalyticPrimitive:
                return AnalyticPrimitive::Count;
            case VolumetricPrimitive:
                return VolumetricPrimitive::Count;
            case SignedDistancePrimitive:
                return SignedDistancePrimitive::Count;
            default:
                return 0;
            }
        }

        static constexpr uint32_t TotalPrimitiveCount = AnalyticPrimitive::Count + VolumetricPrimitive::Count + SignedDistancePrimitive::Count;
    } // namespace IntersectionShaderType

    static constexpr float c_aabbWidth    = 1.0f;
    static constexpr float c_aabbDistance = 1.0f;

    struct RayGenConstantBuffer
    {
        XMFLOAT4 viewport;
        XMFLOAT4 stencil;
    };

    struct PrimitiveInstanceConstantBuffer
    {
        uint32_t instanceIndex;
        uint32_t primitiveType;
    };

    struct PrimitiveConstantBuffer
    {
        XMFLOAT4 albedo;
        float    reflectanceCoef;
        float    diffuseCoef;
        float    specularCoef;
        float    specularPower;
        float    stepScale;

        XMFLOAT3 padding;
    };

    struct ProceduralPrimitiveAttributes
    {
        XMFLOAT3 normal;
    };

    class RayTracedProceduralGeometryExample final : public IExample, public NodeExecutionCallback, public PresentExecutionCallback
    {
        std::unique_ptr<RenderGraph>                 m_renderGraph;
        std::unique_ptr<ICommandListPool>            m_commandListPool;
        std::unique_ptr<NodeExecutionCallbackHolder> m_copyToPresentCallback;

        // Ray tracing resources
        std::unique_ptr<ITextureResource> m_raytracingOutput[ 3 ];
        std::unique_ptr<IBufferResource>  m_vertexBuffer;
        std::unique_ptr<IBufferResource>  m_indexBuffer;
        std::unique_ptr<IBufferResource>  m_aabbBuffer;
        std::unique_ptr<IBufferResource>  m_rayGenCBResource;
        std::unique_ptr<IBufferResource>  m_aabbPrimitiveAttributeBuffer;
        void                             *m_aabbPrimitiveAttributeBufferMemory = nullptr;
        // Scene
        PrimitiveConstantBuffer                        m_planeMaterialCB{ };
        PrimitiveConstantBuffer                        m_aabbMaterialCB[ IntersectionShaderType::TotalPrimitiveCount ] = { };
        std::vector<PrimitiveConstantBuffer>           m_aabbMaterials;
        SceneConstantBuffer                            m_sceneConstants{ };
        std::unique_ptr<IBufferResource>               m_sceneConstantBuffer;
        std::vector<InteropArray<InteropArray<float>>> m_aabbTransformsPerFrame;
        // Camera:
        XMVECTOR m_eye{ };
        XMVECTOR m_at{ };
        XMVECTOR m_up{ };

        // Acceleration Structures
        std::unique_ptr<IBottomLevelAS> m_triangleAS;
        std::unique_ptr<IBottomLevelAS> m_aabbAS;
        std::unique_ptr<ITopLevelAS>    m_topLevelAS;

        // Pipeline objects
        std::unique_ptr<ShaderProgram>      m_rayTracingProgram;
        std::unique_ptr<IRootSignature>     m_rayTracingRootSignature;
        std::unique_ptr<IPipeline>          m_rayTracingPipeline;
        std::unique_ptr<IResourceBindGroup> m_rayTracingBindGroups[ 3 ];

        // Shader binding table and layouts
        int32_t                                              m_closestHitAABBIndex          = 0;
        int32_t                                              m_firstIntersectionShaderIndex = 0;
        std::unique_ptr<IShaderBindingTable>                 m_shaderBindingTable;
        std::vector<std::unique_ptr<IShaderLocalDataLayout>> m_shaderLocalDataLayouts;
        std::unique_ptr<IShaderLocalData>                    m_hitGroupData;

        // Constants and state
        RayGenConstantBuffer               m_rayGenCB{ };
        std::vector<D3D12_RAYTRACING_AABB> m_aabbs;
        Time                               m_time;
        float                              m_animateGeometryTime = 0.0f;
        bool                               m_animateGeometry     = true;

        void CreateRenderTargets( );
        void CreateResources( );
        void CreateAccelerationStructures( );
        void CreateRayTracingPipeline( );
        void InitializeScene( BatchResourceCopy &batchResourceCopy );
        void UpdateCameraMatrices( );
        void CreateShaderBindingTable( );
        void BuildProceduralGeometryAABBs( );

    public:
        void Init( ) override;
        void ModifyApiPreferences( APIPreference &defaultApiPreference ) override;
        void Update( ) override;
        void Execute( uint32_t frameIndex, ICommandList *commandList ) override;
        void Execute( uint32_t frameIndex, ICommandList *commandList, ITextureResource *renderTarget ) override;
        void HandleEvent( SDL_Event &event ) override;
        void UpdateAABBPrimitiveAttributes( float animationTime ) const;
        ~RayTracedProceduralGeometryExample( ) override;
    };
} // namespace DenOfIz
