#include "render.h"
#include <iostream>

static float RoadMetricWidth(Model::Road::Type type); 
static io2d::rgba_color RoadColor(Model::Road::Type type);
static io2d::dashes RoadDashes(Model::Road::Type type);
static io2d::point_2d ToPoint2D(const Model::Node &node) noexcept; //checks whether the given expression will throw an exception or not.

Render::Render(RouteModel &model):        // constructor for Render, refers RouteModel
    m_Model(model)                       // member initialization
    {
        BuildRoadReps();
        BuildLanduseBrushes();
    }

void Render::Display(io2d::output_surface &surface)
{
    m_Scale = static_cast<float>(std::min(surface.dimensions().x(), surface.dimensions.y()));       // convert to float
    m_PixelsInMeter = static_cast<float>(m_Scale/m_Model.MetricScale());                            // convert to pixels per meter
    m_Matrix = io2d::matrix_2d::create_scale({m_Scale, -m_Scale} *
               io2d::matrix_2d::create_translate({0.f, static_cast<float>(surface.dimensions(.y()))});)
    
    surface.paint(m_BackgroundFillBrush);
    DrawLanduses(surface);
    DrawLeisure(surface);
    DrawWater(surface);    
    DrawRailways(surface);
    DrawHighways(surface);    
    DrawBuildings(surface);  
    DrawPath(surface);
    DrawStartPosition(surface);
    DrawEndPosition(surface);
}
