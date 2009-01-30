// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#include <FL/gl.h>
#include "drawContext.h"
#include "PView.h"
#include "PViewOptions.h"
#include "PViewData.h"
#include "Context.h"
#include "gl2ps.h"

static void drawScaleBar(PView *p, double xmin, double ymin, double width, 
                         double height, double tic, int horizontal)
{
  PViewOptions *opt = p->getOptions();

  double box = (horizontal ? width : height) / (opt->NbIso ? opt->NbIso : 1);

  for(int i = 0; i < opt->NbIso; i++) {
    if(opt->IntervalsType == PViewOptions::Discrete ||
       opt->IntervalsType == PViewOptions::Numeric){
      unsigned int col = opt->getColor(i, opt->NbIso);
      glColor4ubv((GLubyte *) &col);
      glBegin(GL_QUADS);
      if(horizontal){
        glVertex2d(xmin + i * box, ymin);
        glVertex2d(xmin + (i + 1) * box, ymin);
        glVertex2d(xmin + (i + 1) * box, ymin + height);
        glVertex2d(xmin + i * box, ymin + height);
      }
      else{
        glVertex2d(xmin, ymin + i * box);
        glVertex2d(xmin + width, ymin + i * box);
        glVertex2d(xmin + width, ymin + (i + 1) * box);
        glVertex2d(xmin, ymin + (i + 1) * box);
      }
      glEnd();
    }
    else if(opt->IntervalsType == PViewOptions::Continuous){
      glBegin(GL_QUADS);
      double dv = (opt->TmpMax - opt->TmpMin) / (opt->NbIso ? opt->NbIso : 1);
      double v1 = opt->TmpMin + i * dv;
      unsigned int col1 = opt->getColor(v1, opt->TmpMin, opt->TmpMax, true);
      glColor4ubv((GLubyte *) &col1);
      if(horizontal){
        glVertex2d(xmin + i * box, ymin + height);
        glVertex2d(xmin + i * box, ymin);
      }
      else{
        glVertex2d(xmin, ymin + i * box);
        glVertex2d(xmin + width, ymin + i * box);
      }
      double v2 = opt->TmpMin + (i + 1) * dv;
      unsigned int col2 = opt->getColor(v2, opt->TmpMin, opt->TmpMax, true);
      glColor4ubv((GLubyte *) &col2);
      if(horizontal){
        glVertex2d(xmin + (i + 1) * box, ymin);
        glVertex2d(xmin + (i + 1) * box, ymin + height);
      }
      else{
        glVertex2d(xmin + width, ymin + (i + 1) * box);
        glVertex2d(xmin, ymin + (i + 1) * box);
      }
      glEnd();
    }
    else{
      unsigned int col = opt->getColor(i, opt->NbIso);
      glColor4ubv((GLubyte *) &col);
      glBegin(GL_LINES);
      if(horizontal){
        glVertex2d(xmin + box / 2. + i * box, ymin);
        glVertex2d(xmin + box / 2. + i * box, ymin + height);
      }
      else{
        glVertex2d(xmin, ymin + box / 2. + i * box);
        glVertex2d(xmin + width, ymin + box / 2. + i * box);
      }
      glEnd();
    }
  }
}

static void drawScaleValues(drawContext *ctx, PView *p, double xmin, double ymin,
                            double width, double height, double tic, int horizontal)
{
  PViewOptions *opt = p->getOptions();

  if(!opt->NbIso) return;

  gl_font(CTX::instance()->gl_font_enum, CTX::instance()->gl_fontsize);
  double font_h = gl_height(); // total font height
  double font_a = gl_height() - gl_descent(); // height above ref pt

  char label[1024];
  sprintf(label, opt->Format.c_str(), -M_PI * 1.e-4);
  double maxw = gl_width(label);

  int nbv = opt->NbIso;
  double f = (opt->IntervalsType == PViewOptions::Discrete ||
              opt->IntervalsType == PViewOptions::Numeric ||
              opt->IntervalsType == PViewOptions::Continuous) ? 2 : 2.5;

  if(horizontal && width < nbv * maxw){
    if(width < f * maxw) nbv = 1;
    else nbv = 2;
  }
  else if(!horizontal && height < nbv * font_h){
    if(height < f * font_h) nbv = 1;
    else nbv = 2;
  }

  double box = (horizontal ? width : height) / opt->NbIso;
  double vbox = (horizontal ? width : height) / nbv;

  glColor4ubv((GLubyte *) & CTX::instance()->color.text);

  if(opt->IntervalsType == PViewOptions::Discrete ||
     opt->IntervalsType == PViewOptions::Numeric ||
     opt->IntervalsType == PViewOptions::Continuous){
    for(int i = 0; i < nbv + 1; i++) {
      double v = opt->getScaleValue(i, nbv + 1, opt->TmpMin, opt->TmpMax);
      sprintf(label, opt->Format.c_str(), v);
      if(horizontal){
        glRasterPos2d(xmin + i * vbox, ymin + height + tic);
        ctx->drawStringCenter(label);
      }
      else{
        glRasterPos2d(xmin + width + tic, ymin + i * vbox - font_a / 3.);
        ctx->drawString(label);
      }
    }
  }
  else{
    if(opt->NbIso > 2 && (nbv == 1 || nbv == 2)){
      vbox = (vbox * nbv - box) / nbv;
      nbv++;
    }
    for(int i = 0; i < nbv; i++) {
      double v = opt->getScaleValue(i, nbv, opt->TmpMin, opt->TmpMax);
      sprintf(label, opt->Format.c_str(), v);
      if(horizontal){
        glRasterPos2d(xmin + box / 2. + i * vbox, ymin + height + tic);
        ctx->drawStringCenter(label);
      }
      else{
        glRasterPos2d(xmin + width + tic, ymin + box / 2. + i * vbox - font_a / 3.);
        ctx->drawString(label);
      }
    }
  }
}

static void drawScaleLabel(drawContext *ctx, PView *p, double xmin, double ymin, 
                           double width, double height, double tic, int horizontal)
{
  PViewData *data = p->getData();
  PViewOptions *opt = p->getOptions();

  gl_font(CTX::instance()->gl_font_enum, CTX::instance()->gl_fontsize);
  double font_h = gl_height();

  char label[1024];

  int nt = data->getNumTimeSteps();
  if((opt->ShowTime == 1 && nt > 1) || opt->ShowTime == 2){
    char tmp[256];
    sprintf(tmp, opt->Format.c_str(), data->getTime(opt->TimeStep));
    sprintf(label, "%s (%s)", data->getName().c_str(), tmp);
  }
  else if((opt->ShowTime == 3 && nt > 1) || opt->ShowTime == 4){
    sprintf(label, "%s (%d)", data->getName().c_str(), opt->TimeStep);
  }
  else
    sprintf(label, "%s", data->getName().c_str());
 
  if(horizontal){
    glRasterPos2d(xmin + width / 2., ymin + height + tic + 1.4 * font_h);
    ctx->drawStringCenter(label);
  }
  else{
    glRasterPos2d(xmin, ymin - 2 * font_h);
    ctx->drawString(label);
  }
}

static void drawScale(drawContext *ctx, PView *p, double xmin, double ymin,
                      double width, double height, double tic, int horizontal)
{
  // use adaptive data if available
  PViewData *data = p->getData(true);
  PViewOptions *opt = p->getOptions();

  if(opt->ExternalViewIndex >= 0){
    opt->TmpMin = opt->ExternalMin;
    opt->TmpMax = opt->ExternalMax;
  }
  else if(opt->RangeType == PViewOptions::Custom){
    opt->TmpMin = opt->CustomMin;
    opt->TmpMax = opt->CustomMax;
  }
  else if(opt->RangeType == PViewOptions::PerTimeStep){
    opt->TmpMin = data->getMin(opt->TimeStep);
    opt->TmpMax = data->getMax(opt->TimeStep);
  }
  else{
    opt->TmpMin = data->getMin();
    opt->TmpMax = data->getMax();
  }

  drawScaleBar(p, xmin, ymin, width, height, tic, horizontal);
  drawScaleValues(ctx, p, xmin, ymin, width, height, tic, horizontal);
  drawScaleLabel(ctx, p, xmin, ymin, width, height, tic, horizontal);
}

void drawContext::drawScales()
{
  std::vector<PView*> scales;
  for(unsigned int i = 0; i < PView::list.size(); i++){
    PViewData *data = PView::list[i]->getData();
    PViewOptions *opt = PView::list[i]->getOptions();
    if(!data->getDirty() && opt->Visible && opt->ShowScale && 
       opt->Type == PViewOptions::Plot3D && data->getNumElements() &&
       isVisible(PView::list[i]))
      scales.push_back(PView::list[i]);
  }
  if(scales.empty()) return;

  gl_font(CTX::instance()->gl_font_enum, CTX::instance()->gl_fontsize);
  char label[1024];
  double maxw = 0.;
  for(unsigned int i = 0; i < scales.size(); i++) {
    PViewOptions *opt = scales[i]->getOptions();
    sprintf(label, opt->Format.c_str(), -M_PI * 1.e-4);
    maxw = std::max(maxw, gl_width(label));
  }

  const double tic = 10., bar_size = 16.;
  double width = 0., width_prev = 0., width_total = 0.;

  for(unsigned int i = 0; i < scales.size(); i++) {
    PView *p = scales[i];
    PViewData *data = p->getData();
    PViewOptions *opt = p->getOptions();
    
    if(!opt->AutoPosition) {
      double w = opt->Size[0], h = opt->Size[1];
      double x = opt->Position[0], y = opt->Position[1] - h;
      int c = fix2dCoordinates(&x, &y);
      if(c & 1) x -= w / 2.;
      if(c & 2) y += h / 2.;
      drawScale(this, p, x, y, w, h, tic, CTX::instance()->post.horizontal_scales);
    }
    else if(CTX::instance()->post.horizontal_scales){
      double ysep = 20.;
      double xc = (viewport[2] - viewport[0]) / 2.;
      if(scales.size() == 1){
        double w = (viewport[2] - viewport[0]) / 2., h = bar_size;
        double x = xc - w / 2., y = viewport[1] + ysep;
        drawScale(this, p, x, y, w, h, tic, 1);
      }
      else{
        double xsep = maxw / 4. + (viewport[2] - viewport[0]) / 10.;
        double w = (viewport[2] - viewport[0] - 4 * xsep) / 2.;
        if(w < 20.) w = 20.;
        double h = bar_size;
        double x = xc - (i % 2 ? -xsep / 1.5 : w + xsep / 1.5);
        double y = viewport[1] + ysep + 
          (i / 2) * (bar_size + tic + 2 * gl_height() + ysep);
        drawScale(this, p, x, y, w, h, tic, 1);
      }
    }
    else{
      double xsep = 20.;
      double dy = 2. * gl_height();
      if(scales.size() == 1){
        double ysep = (viewport[3] - viewport[1]) / 6.;
        double w = bar_size, h = viewport[3] - viewport[1] - 2 * ysep - dy;
        double x = viewport[0] + xsep, y = viewport[1] + ysep + dy;
        drawScale(this, p, x, y, w, h, tic, 0);
      }
      else{
        double ysep = (viewport[3] - viewport[1]) / 15.;
        double w = bar_size;
        double h = (viewport[3] - viewport[1] - 3 * ysep - 2.5 * dy) / 2.;
        double x = viewport[0] + xsep + width_total + (i / 2) * xsep;
        double y = viewport[1] + ysep + dy + (1 - i % 2) * (h + 1.5 * dy + ysep);
        drawScale(this, p, x, y, w, h, tic, 0);
      }
      // compute width
      width_prev = width;
      sprintf(label, opt->Format.c_str(), -M_PI * 1.e-4);
      width = bar_size + tic + gl_width(label);
      if(opt->ShowTime){
        char tmp[256];
        sprintf(tmp, opt->Format.c_str(), data->getTime(opt->TimeStep));
        sprintf(label, "%s (%s)", data->getName().c_str(), tmp);
      }
      else
        sprintf(label, "%s", data->getName().c_str());
      width = std::max(width, gl_width(label));
      if(i % 2) width_total += std::max(bar_size + width, bar_size + width_prev);
    }
  }
}
