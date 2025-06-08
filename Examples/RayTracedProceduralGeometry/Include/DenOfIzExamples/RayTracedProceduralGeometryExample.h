#pragma once

#include "DenOfIzExamples/IExample.h"
#include "DenOfIzGraphics/Utilities/StepTimer.h"
#ifndef UINT
#define UINT uint32_t
#endif
#include <RayTracingHlslCompat.h>

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

    class RayTracedProceduralGeometryExample final : public IExample
    {
        static constexpr UINT  NUM_BLAS       = 2;
        static constexpr float c_aabbWidth    = 2.0f;
        static constexpr float c_aabbDistance = 2.0f;

        std::vector<std::unique_ptr<ICommandList>> m_commandLists;

        // Ray tracing resources
        std::unique_ptr<ITextureResource> m_raytracingOutput[ 3 ];
        std::unique_ptr<IBufferResource>  m_vertexBuffer;
        std::unique_ptr<IBufferResource>  m_indexBuffer;
        std::unique_ptr<IBufferResource>  m_aabbBuffer;
        std::unique_ptr<IBufferResource>  m_aabbPrimitiveAttributeBuffer;
        PrimitiveInstancePerFrameBuffer  *m_aabbPrimitiveAttributeBufferMemory = nullptr;
        // Scene
        PrimitiveConstantBuffer                        m_planeMaterialCB{ };
        std::vector<PrimitiveConstantBuffer>           m_aabbMaterials;
        std::unique_ptr<IBufferResource>               m_sceneConstantBuffer;
        SceneConstantBuffer                           *m_sceneConstants = nullptr;
        std::vector<InteropArray<InteropArray<float>>> m_aabbTransformsPerFrame;

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
        int32_t                              m_closestHitTriangleIndex      = 0;
        int32_t                              m_closestHitAABBIndex          = 0;
        int32_t                              m_firstIntersectionShaderIndex = 0;
        std::unique_ptr<IShaderBindingTable> m_shaderBindingTable;
        std::unique_ptr<ILocalRootSignature> m_hgLocalRootSignature;
        std::unique_ptr<IShaderLocalData>    m_hitGroupData;

        // Constants and state
        std::vector<AABBBoundingBox>               m_aabbs;
        std::vector<InteropArray<AABBBoundingBox>> m_aabbPerGeometry;
        double                                     m_animateGeometryTime = 1.0f;
        bool                                       m_animateGeometry     = true;

        void CreateRenderTargets( );
        void CreateResources( );
        void CreateAccelerationStructures( );
        void CreateRayTracingPipeline( );
        void InitializeScene( );
        void InitCamera( ) const;
        void CreateShaderBindingTable( );
        void BuildProceduralGeometryAABBs( );

    public:
        void Init( ) override;
        void ModifyApiPreferences( APIPreference &defaultApiPreference ) override;
        void Update( ) override;
        void Render( uint32_t frameIndex, ICommandList *commandList ) override;
        void HandleEvent( Event &event ) override;
        void UpdateAABBPrimitiveAttributes( );
        void Quit( ) override;
        ~RayTracedProceduralGeometryExample( ) override = default;
    };
} // namespace DenOfIz
