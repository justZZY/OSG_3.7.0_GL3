/* OpenSceneGraph example, osgpersistentbufferstorage
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

/* file:        examples/osgpersistentbufferstorage/osgpersistentbufferstorage.cpp
* author:      Julien Valentin 2020-10-01
* copyright:   (C) 2013
* license:     OpenSceneGraph Public License (OSGPL)
*
* A test of BufferStorage and PersistantBufferStorage features
*
*/


#include <osg/GL2Extensions>
#include <osg/Notify>
#include <osg/ref_ptr>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Point>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Program>
#include <osg/Shader>
#include <osg/BlendFunc>

#include <osg/Uniform>
#include <osgViewer/Viewer>

#include <osg/BufferIndexBinding>

#include <iostream>

///////////////////////////////////////////////////////////////////////////

class SineAnimation: public osg::Camera::DrawCallback
{
public:
    SineAnimation(osg::Vec4Array* dyn, float scale = 1.0f, float offset = 0.0f ) :
        _dyn(dyn),_rate(0), _scale(scale), _offset(offset)
    {}

    void operator()(osg::RenderInfo& renderInfo) const
    {
        unsigned int contextId =  renderInfo.getContextID();
        osg::GLBufferObject * glbo = _dyn->getBufferObject()->getOrCreateGLBufferObject(contextId);
        glbo->bindBuffer();
        GLfloat * data=(GLfloat*)glbo->_persistentDMA+glbo->getOffset(_dyn->getBufferIndex());
        if(data)
        {
            _rate+=0.01;
            float angle = _rate ;
            float value =  sinf( angle ) * _scale + _offset;
            for(int i=0; i<4; i++) {
                data[i*4+0]=float(i)*0.25*value;
            }
            glbo->commitDMA(_dyn->getBufferIndex());
        }
    }

private:
    osg::ref_ptr<osg::Vec4Array> _dyn;
    mutable float _rate;
    const float _scale;
    const float _offset;
};


///////////////////////////////////////////////////////////////////////////

int main( int, char** )
{
    osg::Camera* root=new osg::Camera;
    /// a first geom to demonstrate how to use usage to enable immutable buffer storage
    {
        osg::Vec4Array* vAry = new osg::Vec4Array;
        vAry->push_back( osg::Vec4(2,0,0,1) );
        vAry->push_back( osg::Vec4(2,1,0,1) );
        vAry->push_back( osg::Vec4(3,0,0,1) );
        vAry->push_back( osg::Vec4(3,1,0,1 ));
        osg::VertexBufferObject*vbo = new osg::VertexBufferObject;
        vbo->setUsage(GL_MAP_WRITE_BIT);//enable bufferStorage
        vbo->setMappingBitfield(GL_MAP_WRITE_BIT);//set mapping flags
        vAry->setBufferObject(vbo);
        osg::Geometry * geom=new osg::Geometry;
        geom->setUseDisplayList(false);
        geom->setUseVertexBufferObjects(true);
        geom->setVertexArray( vAry );
        geom->addPrimitiveSet( new osg::DrawArrays( GL_LINES, 0, vAry->size() ) );
        root->addChild(geom);
    }

    //second geometry buffer is persistent mapped and modified in camera callback
    {
        osg::Vec4Array* vAry = new osg::Vec4Array;
        vAry->setDataVariance(osg::Object::STATIC);
        vAry->push_back( osg::Vec4(0,0,0,1) );
        vAry->push_back( osg::Vec4(0,1,0,1) );
        vAry->push_back( osg::Vec4(1,0,0,1) );
        vAry->push_back( osg::Vec4(1,1,0,1 ));
        osg::VertexBufferObject*vbo = new osg::VertexBufferObject;
        vbo->setDataVariance(osg::Object::STATIC);
        vbo->setUsage(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT );//enable persistant bufferStorage
        vbo->setMappingBitfield(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT| GL_MAP_FLUSH_EXPLICIT_BIT );//set mapping flags

        vAry->setBufferObject(vbo);
        osg::Geometry * geom=new osg::Geometry;

        geom->setUseDisplayList(false);
        geom->setUseVertexBufferObjects(true);
        geom->setVertexArray( vAry );
        geom->addPrimitiveSet( new osg::DrawArrays( GL_QUADS, 0, vAry->size() ) );
        root->addChild(geom);

        root->addPostDrawCallback(new SineAnimation(vAry));
    }
    osgViewer::Viewer viewer;
    viewer.setSceneData( root );
    return viewer.run();
}
