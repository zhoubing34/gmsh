// Gmsh - Copyright (C) 1997-2011 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#include "GmshConfig.h"
#include "GmshMessage.h"
#include "GModel.h"
#include "GmshDefines.h"
#include "StringUtils.h"
#include "Context.h"
#include "Options.h"
#include "OS.h"

#if defined(HAVE_OPENGL)
#include "drawContext.h"
#endif

#if defined(HAVE_FLTK)
#include "FlGui.h"
#include "graphicWindow.h"
#include "gl2ps.h"
#include "gl2gif.h"
#include "gl2jpeg.h"
#include "gl2png.h"
#include "gl2ppm.h"
#include "gl2yuv.h"
#endif

int GuessFileFormatFromFileName(std::string fileName)
{
  std::string ext = SplitFileName(fileName)[2];
  if     (ext == ".geo")  return FORMAT_GEO;
  else if(ext == ".msh")  return FORMAT_MSH;
  else if(ext == ".pos")  return FORMAT_POS;
  else if(ext == ".opt")  return FORMAT_OPT;
  else if(ext == ".unv")  return FORMAT_UNV;
  else if(ext == ".vtk")  return FORMAT_VTK;
  else if(ext == ".txt")  return FORMAT_TXT;
  else if(ext == ".stl")  return FORMAT_STL;
  else if(ext == ".cgns") return FORMAT_CGNS;
  else if(ext == ".med")  return FORMAT_MED;
  else if(ext == ".rmed") return FORMAT_RMED;
  else if(ext == ".ir3")  return FORMAT_IR3;
  else if(ext == ".mesh") return FORMAT_MESH;
  else if(ext == ".mail") return FORMAT_MAIL;
  else if(ext == ".bdf")  return FORMAT_BDF;
  else if(ext == ".diff") return FORMAT_DIFF;
  else if(ext == ".inp")  return FORMAT_INP;
  else if(ext == ".nas")  return FORMAT_BDF;
  else if(ext == ".p3d")  return FORMAT_P3D;
  else if(ext == ".wrl")  return FORMAT_VRML;
  else if(ext == ".vrml") return FORMAT_VRML;
  else if(ext == ".ply2") return FORMAT_PLY2;
  else if(ext == ".gif")  return FORMAT_GIF;
  else if(ext == ".jpg")  return FORMAT_JPEG;
  else if(ext == ".jpeg") return FORMAT_JPEG;
  else if(ext == ".mpg")  return FORMAT_MPEG;
  else if(ext == ".mpeg") return FORMAT_MPEG;
  else if(ext == ".png")  return FORMAT_PNG;
  else if(ext == ".ps")   return FORMAT_PS;
  else if(ext == ".eps")  return FORMAT_EPS;
  else if(ext == ".pdf")  return FORMAT_PDF;
  else if(ext == ".tex")  return FORMAT_TEX;
  else if(ext == ".svg")  return FORMAT_SVG;
  else if(ext == ".ppm")  return FORMAT_PPM;
  else if(ext == ".yuv")  return FORMAT_YUV;
  else if(ext == ".brep") return FORMAT_BREP;
  else if(ext == ".step") return FORMAT_STEP;
  else if(ext == ".stp")  return FORMAT_STEP;
  else if(ext == ".iges") return FORMAT_IGES;
  else if(ext == ".igs")  return FORMAT_IGES;
  else                           return -1;
}

std::string GetDefaultFileName(int format)
{
  std::vector<std::string> split = SplitFileName(GModel::current()->getFileName());
  std::string name = split[0] + split[1];
  switch(format){
  case FORMAT_GEO:  name += ".geo_unrolled"; break;
  case FORMAT_MSH:  name += ".msh"; break;
  case FORMAT_POS:  name += ".pos"; break;
  case FORMAT_OPT:  name += ".opt"; break;
  case FORMAT_UNV:  name += ".unv"; break;
  case FORMAT_VTK:  name += ".vtk"; break;
  case FORMAT_STL:  name += ".stl"; break;
  case FORMAT_CGNS: name += ".cgns"; break;
  case FORMAT_MED:  name += ".med"; break;
  case FORMAT_RMED: name += ".rmed"; break;
  case FORMAT_IR3:  name += ".ir3"; break;
  case FORMAT_MESH: name += ".mesh"; break;
  case FORMAT_MAIL: name += ".mail"; break;
  case FORMAT_BDF:  name += ".bdf"; break;
  case FORMAT_DIFF: name += ".diff"; break;
  case FORMAT_INP:  name += ".inp"; break;
  case FORMAT_P3D:  name += ".p3d"; break;
  case FORMAT_VRML: name += ".wrl"; break;
  case FORMAT_PLY2: name += ".ply2"; break;
  case FORMAT_GIF:  name += ".gif"; break;
  case FORMAT_JPEG: name += ".jpg"; break;
  case FORMAT_MPEG: name += ".mpg"; break;
  case FORMAT_PNG:  name += ".png"; break;
  case FORMAT_PS:   name += ".ps"; break;
  case FORMAT_EPS:  name += ".eps"; break;
  case FORMAT_PDF:  name += ".pdf"; break;
  case FORMAT_TEX:  name += ".tex"; break;
  case FORMAT_SVG:  name += ".svg"; break;
  case FORMAT_PPM:  name += ".ppm"; break;
  case FORMAT_YUV:  name += ".yuv"; break;
  case FORMAT_BREP: name += ".brep"; break;
  case FORMAT_IGES: name += ".iges"; break;
  case FORMAT_STEP: name += ".step"; break;
  default: break;
  }
  return name;
}

#if defined(HAVE_FLTK)
static PixelBuffer *GetCompositePixelBuffer(GLenum format, GLenum type)
{
  PixelBuffer *buffer;
  if(!CTX::instance()->print.compositeWindows){
    GLint width = FlGui::instance()->getCurrentOpenglWindow()->w();
    GLint height = FlGui::instance()->getCurrentOpenglWindow()->h();
    buffer = new PixelBuffer(width, height, format, type);
    buffer->fill(CTX::instance()->batch);
  }
  else{
    graphicWindow *g = FlGui::instance()->graph[0];
    for(unsigned int i = 1; i < FlGui::instance()->graph.size(); i++){
      for(unsigned int j = 0; j < FlGui::instance()->graph[i]->gl.size(); j++){
        if(FlGui::instance()->graph[i]->gl[j] == 
           FlGui::instance()->getCurrentOpenglWindow()){
          g = FlGui::instance()->graph[i];
          break;
        }
      }
    }
    int ww = 0, hh = 0;
    std::vector<PixelBuffer*> buffers;
    for(unsigned int i = 0; i < g->gl.size(); i++){
      openglWindow::setLastHandled(g->gl[i]);
      buffer = new PixelBuffer(g->gl[i]->w(), g->gl[i]->h(), format, type);
      buffer->fill(CTX::instance()->batch);
      buffers.push_back(buffer);
      ww = std::max(ww, g->gl[i]->x() + g->gl[i]->w());
      hh = std::max(hh, g->gl[i]->y() + g->gl[i]->h());
    }
    buffer = new PixelBuffer(ww, hh, format, type);
    for(unsigned int i = 0; i < g->gl.size(); i++){
      buffer->copyPixels(g->gl[i]->x(), hh - g->gl[i]->h() - g->gl[i]->y(), 
                         buffers[i]);
      delete buffers[i];
    }
  }
  return buffer;
}
#endif

void CreateOutputFile(std::string fileName, int format)
{
  if(fileName.empty())
    fileName = GetDefaultFileName(format);

  int oldFormat = CTX::instance()->print.fileFormat;
  CTX::instance()->print.fileFormat = format;
  CTX::instance()->printing = 1;

  if(format != FORMAT_AUTO) 
    Msg::StatusBar(2, true, "Writing '%s'...", fileName.c_str());

  bool printEndMessage = true;

  switch (format) {

  case FORMAT_AUTO:
    CreateOutputFile(fileName, GuessFileFormatFromFileName(fileName));
    printEndMessage = false;
    break;
    
  case FORMAT_OPT:
    PrintOptions(0, GMSH_FULLRC, 1, 1, fileName.c_str());
    break;

  case FORMAT_MSH:
    if(GModel::current()->getMeshPartitions().size() && 
       CTX::instance()->mesh.mshFilePartitioned){
      GModel::current()->writePartitionedMSH
        (fileName, CTX::instance()->mesh.binary, CTX::instance()->mesh.saveAll,
         CTX::instance()->mesh.saveParametric, CTX::instance()->mesh.scalingFactor);
    }
    else{
      GModel::current()->writeMSH
        (fileName, CTX::instance()->mesh.mshFileVersion,
         CTX::instance()->mesh.binary, CTX::instance()->mesh.saveAll,
         CTX::instance()->mesh.saveParametric, CTX::instance()->mesh.scalingFactor);
    }
    break;

  case FORMAT_STL:
    GModel::current()->writeSTL
      (fileName, CTX::instance()->mesh.binary, CTX::instance()->mesh.saveAll,
       CTX::instance()->mesh.scalingFactor);
    break;

  case FORMAT_VRML:
    GModel::current()->writeVRML
      (fileName, CTX::instance()->mesh.saveAll, CTX::instance()->mesh.scalingFactor);
    break;

  case FORMAT_PLY2:
    GModel::current()->writePLY2(fileName);
    break;

  case FORMAT_UNV:
    GModel::current()->writeUNV
      (fileName, CTX::instance()->mesh.saveAll, CTX::instance()->mesh.saveGroupsOfNodes,
       CTX::instance()->mesh.scalingFactor);
    break;

  case FORMAT_VTK:
    GModel::current()->writeVTK
      (fileName, CTX::instance()->mesh.binary, CTX::instance()->mesh.saveAll,
       CTX::instance()->mesh.scalingFactor,
       CTX::instance()->bigEndian);
    break;

  case FORMAT_MESH:
    GModel::current()->writeMESH
      (fileName, CTX::instance()->mesh.saveElementTagType, 
       CTX::instance()->mesh.saveAll, CTX::instance()->mesh.scalingFactor);
    break;

  case FORMAT_MAIL:
    GModel::current()->writeMAIL
      (fileName, CTX::instance()->mesh.saveAll, CTX::instance()->mesh.scalingFactor);
    break;

  case FORMAT_IR3:
    GModel::current()->writeIR3
      (fileName, CTX::instance()->mesh.saveElementTagType, 
       CTX::instance()->mesh.saveAll, CTX::instance()->mesh.scalingFactor);
    break;

  case FORMAT_BDF:
    GModel::current()->writeBDF
      (fileName, CTX::instance()->mesh.bdfFieldFormat, 
       CTX::instance()->mesh.saveElementTagType, CTX::instance()->mesh.saveAll,
       CTX::instance()->mesh.scalingFactor);
    break;

  case FORMAT_DIFF:
    GModel::current()->writeDIFF
      (fileName, CTX::instance()->mesh.binary, CTX::instance()->mesh.saveAll, 
       CTX::instance()->mesh.scalingFactor);
    break;

  case FORMAT_INP:
    GModel::current()->writeINP
      (fileName, CTX::instance()->mesh.saveAll, CTX::instance()->mesh.scalingFactor);
    break;

  case FORMAT_P3D:
    GModel::current()->writeP3D
      (fileName, CTX::instance()->mesh.saveAll, CTX::instance()->mesh.scalingFactor);
    break;

  case FORMAT_CGNS:
    GModel::current()->writeCGNS
      (fileName, CTX::instance()->mesh.zoneDefinition, CTX::instance()->cgnsOptions, 
       CTX::instance()->mesh.scalingFactor);
    break;

  case FORMAT_MED:
    GModel::current()->writeMED
      (fileName, CTX::instance()->mesh.saveAll, CTX::instance()->mesh.scalingFactor);
    break;

  case FORMAT_POS:
    GModel::current()->writePOS
      (fileName, CTX::instance()->print.posElementary, 
       CTX::instance()->print.posElement, CTX::instance()->print.posGamma,
       CTX::instance()->print.posEta, CTX::instance()->print.posRho,
       CTX::instance()->print.posDisto, CTX::instance()->mesh.saveAll,
       CTX::instance()->mesh.scalingFactor);
    break;

  case FORMAT_GEO:
    GModel::current()->writeGEO(fileName, CTX::instance()->print.geoLabels);
    break;

  case FORMAT_BREP:
    GModel::current()->writeOCCBREP(fileName);
    break;

  case FORMAT_STEP:
    GModel::current()->writeOCCSTEP(fileName);
    break;

#if defined(HAVE_FLTK)
  case FORMAT_PPM:
  case FORMAT_YUV:
  case FORMAT_GIF:  
  case FORMAT_JPEG:
  case FORMAT_PNG:
    {
      if(!FlGui::available()) break;

      FILE *fp = fopen(fileName.c_str(), "wb");
      if(!fp){
        Msg::Error("Unable to open file '%s'", fileName.c_str());
        break;
      }

      int oldGradient = CTX::instance()->bgGradient;
      if(format == FORMAT_GIF && CTX::instance()->print.gifTransparent)
        CTX::instance()->bgGradient = 0;

      PixelBuffer *buffer = GetCompositePixelBuffer(GL_RGB, GL_UNSIGNED_BYTE);

      CTX::instance()->bgGradient = oldGradient;

      if(format == FORMAT_PPM)
        create_ppm(fp, buffer);
      else if(format == FORMAT_YUV)
        create_yuv(fp, buffer);
      else if(format == FORMAT_GIF)
        create_gif(fp, buffer, 
                   CTX::instance()->print.gifDither,
                   CTX::instance()->print.gifSort,
                   CTX::instance()->print.gifInterlace,
                   CTX::instance()->print.gifTransparent,
                   CTX::instance()->unpackRed(CTX::instance()->color.bg),
                   CTX::instance()->unpackGreen(CTX::instance()->color.bg), 
                   CTX::instance()->unpackBlue(CTX::instance()->color.bg));
      else if(format == FORMAT_JPEG)
        create_jpeg(fp, buffer, CTX::instance()->print.jpegQuality, 
                    CTX::instance()->print.jpegSmoothing);
      else
        create_png(fp, buffer, 100);

      delete buffer;
      fclose(fp);
    }
    break;

  case FORMAT_PS:
  case FORMAT_EPS:
  case FORMAT_PDF:
  case FORMAT_SVG:
    {
      if(!FlGui::available()) break;

      FILE *fp = fopen(fileName.c_str(), "wb");
      if(!fp){
        Msg::Error("Unable to open file '%s'", fileName.c_str());
        break;
      }
      std::string base = SplitFileName(fileName)[1];
      GLint width = FlGui::instance()->getCurrentOpenglWindow()->w();
      GLint height = FlGui::instance()->getCurrentOpenglWindow()->h();
      GLint viewport[4] = {0, 0, width, height};

      int oldGradient = CTX::instance()->bgGradient;
      if(!CTX::instance()->print.epsBackground) CTX::instance()->bgGradient = 0;
      
      PixelBuffer buffer(width, height, GL_RGB, GL_FLOAT);
      
      if(CTX::instance()->print.epsQuality == 0)
        buffer.fill(CTX::instance()->batch);
      
      int psformat = 
        (format == FORMAT_PDF) ? GL2PS_PDF :
        (format == FORMAT_PS) ? GL2PS_PS :
        (format == FORMAT_SVG) ? GL2PS_SVG :
        GL2PS_EPS;
      int pssort = 
        (CTX::instance()->print.epsQuality == 3) ? GL2PS_NO_SORT :
        (CTX::instance()->print.epsQuality == 2) ? GL2PS_BSP_SORT : 
        GL2PS_SIMPLE_SORT;
      int psoptions =
        GL2PS_SIMPLE_LINE_OFFSET | GL2PS_SILENT |
        (CTX::instance()->print.epsOcclusionCulling ? GL2PS_OCCLUSION_CULL : 0) |
        (CTX::instance()->print.epsBestRoot ? GL2PS_BEST_ROOT : 0) |
        (CTX::instance()->print.epsBackground ? GL2PS_DRAW_BACKGROUND : 0) |
        (CTX::instance()->print.epsCompress ? GL2PS_COMPRESS : 0) |
        (CTX::instance()->print.epsPS3Shading ? 0 : GL2PS_NO_PS3_SHADING);

      GLint buffsize = 0;
      int res = GL2PS_OVERFLOW;
      while(res == GL2PS_OVERFLOW) {
        buffsize += 2048 * 2048;
        gl2psBeginPage(base.c_str(), "Gmsh", viewport, 
                       psformat, pssort, psoptions, GL_RGBA, 0, NULL, 
                       15, 20, 10, buffsize, fp, base.c_str());
        if(CTX::instance()->print.epsQuality == 0){
          double modelview[16], projection[16];
          glGetDoublev(GL_PROJECTION_MATRIX, projection);
          glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
          glMatrixMode(GL_PROJECTION);
          glLoadIdentity();
          glOrtho((double)viewport[0], (double)viewport[2],
                  (double)viewport[1], (double)viewport[3], -1., 1.);
          glMatrixMode(GL_MODELVIEW);
          glLoadIdentity();
          glRasterPos2d(0, 0);
          gl2psDrawPixels(width, height, 0, 0, GL_RGB, GL_FLOAT, buffer.getPixels());
          glMatrixMode(GL_PROJECTION);
          glLoadMatrixd(projection);
          glMatrixMode(GL_MODELVIEW);
          glLoadMatrixd(modelview);
        }
        else{
          buffer.fill(CTX::instance()->batch);
        }
        res = gl2psEndPage();
      }

      CTX::instance()->bgGradient = oldGradient;
      fclose(fp);
    }
    break;

  case FORMAT_TEX:
    {
      if(!FlGui::available()) break;

      FILE *fp = fopen(fileName.c_str(), "w");
      if(!fp){
        Msg::Error("Unable to open file '%s'", fileName.c_str());
        break;
      }
      std::string base = SplitFileName(fileName)[1];
      GLint width = FlGui::instance()->getCurrentOpenglWindow()->w();
      GLint height = FlGui::instance()->getCurrentOpenglWindow()->h();
      GLint viewport[4] = {0, 0, width, height};
      GLint buffsize = 0;
      int res = GL2PS_OVERFLOW;
      while(res == GL2PS_OVERFLOW) {
        buffsize += 2048 * 2048;
        gl2psBeginPage(base.c_str(), "Gmsh", viewport,
                       GL2PS_TEX, GL2PS_NO_SORT, GL2PS_NONE, GL_RGBA, 0, NULL, 
                       0, 0, 0, buffsize, fp, base.c_str());
        PixelBuffer buffer(width, height, GL_RGB, GL_UNSIGNED_BYTE);
        int oldtext = CTX::instance()->print.text;
        CTX::instance()->print.text = 1;
        buffer.fill(CTX::instance()->batch);
        CTX::instance()->print.text = oldtext;
        res = gl2psEndPage();
      }
      fclose(fp);
    }
    break;

#if defined(HAVE_MPEG_ENCODE)
  case FORMAT_MPEG:
    {
      std::string parFileName = CTX::instance()->homeDir + ".gmsh-mpeg_encode.par";
      FILE *fp = fopen(parFileName.c_str(), "w");
      if(!fp){
        Msg::Error("Unable to open file '%s'", parFileName.c_str());
        break;
      }
      int numViews = (int)opt_post_nb_views(0, GMSH_GET, 0), numSteps = 0;
      for(int i = 0; i < numViews; i++){
        if(opt_view_visible(i, GMSH_GET, 0))
          numSteps = std::max(numSteps,
                              (int)opt_view_nb_non_empty_timestep(i, GMSH_GET, 0));
      }
      std::vector<std::string> frames;
      for(int i = 0; i < (CTX::instance()->post.animCycle ? numViews : numSteps); i++){
        char tmp[256];
        sprintf(tmp, ".gmsh-%06d.ppm", i);
        frames.push_back(tmp);
      }
      status_play_manual(!CTX::instance()->post.animCycle, 0);
      for(unsigned int i = 0; i < frames.size(); i++){
        CreateOutputFile(CTX::instance()->homeDir + frames[i], FORMAT_PPM);
        status_play_manual(!CTX::instance()->post.animCycle, 1);
      }
      int repeat = (int)(CTX::instance()->post.animDelay * 24);
      if(repeat < 1) repeat = 1;
      std::string pattern("I");
      // including P frames would lead to smaller files, but the
      // quality degradation is perceptible:
      // for(int i = 1; i < repeat; i++) pattern += "P";
      fprintf(fp, "PATTERN %s\nBASE_FILE_FORMAT PPM\nGOP_SIZE %d\n"
              "SLICES_PER_FRAME 1\nPIXEL FULL\nRANGE 10\n"
              "PSEARCH_ALG EXHAUSTIVE\nBSEARCH_ALG CROSS2\n"
              "IQSCALE 1\nPQSCALE 1\nBQSCALE 25\nREFERENCE_FRAME DECODED\n"
              "OUTPUT %s\nINPUT_CONVERT *\nINPUT_DIR %s\nINPUT\n",
              pattern.c_str(), repeat, fileName.c_str(), 
              CTX::instance()->homeDir.c_str());
      for(unsigned int i = 0; i < frames.size(); i++){
        fprintf(fp, "%s", frames[i].c_str());
        if(repeat > 1) fprintf(fp, " [1-%d]", repeat);
        fprintf(fp, "\n");
      }
      fprintf(fp, "END_INPUT\n");
      fclose(fp);
      extern int mpeg_encode_main(int, char**);
      char *args[] = {(char*)"gmsh", (char*)parFileName.c_str()};
      try{
        mpeg_encode_main(2, args);
      }
      catch (const char *error){
        Msg::Error("mpeg_encode: %s", error);
      }
      if(opt_print_delete_tmp_files(0, GMSH_GET, 0)){
        UnlinkFile(parFileName);
        for(unsigned int i = 0; i < frames.size(); i++)
          UnlinkFile(CTX::instance()->homeDir + frames[i]);
      }
    }
    break;
#endif

#endif

  default:
    Msg::Error("Unknown output file format %d", format);
    printEndMessage = false;
    break;
  }

  if(printEndMessage) Msg::StatusBar(2, true, "Done writing '%s'", fileName.c_str());

  CTX::instance()->print.fileFormat = oldFormat;
  CTX::instance()->printing = 0;

#if defined(HAVE_OPENGL)
  drawContext::global()->draw();
#endif
}
