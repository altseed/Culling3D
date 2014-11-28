
#include "Culling3D.WorldInternal.h"
#include "Culling3D.ObjectInternal.h"

namespace Culling3D
{
	World* World::Create(float xSize, float ySize, float zSize, int32_t layerCount)
	{
		return new WorldInternal(xSize, ySize, zSize, layerCount);
	}

	WorldInternal::WorldInternal(float xSize, float ySize, float zSize, int32_t layerCount)
	{
		this->xSize = xSize;
		this->ySize = ySize;
		this->zSize = zSize;

		this->gridSize = Max(Max(this->xSize, this->ySize), this->zSize);

		this->layerCount = layerCount;

		layers.resize(this->layerCount);

		for (size_t i = 0; layers.size(); i++)
		{
			float gridSize_ = this->gridSize / powf(2.0f, (float)i);

			int32_t xCount = (int32_t) (this->xSize / gridSize_);
			int32_t yCount = (int32_t) (this->ySize / gridSize_);
			int32_t zCount = (int32_t) (this->zSize / gridSize_);

			if (xCount * gridSize_ < this->xSize) xCount++;
			if (yCount * gridSize_ < this->ySize) yCount++;
			if (zCount * gridSize_ < this->zSize) zCount++;

			layers[i] = new Layer(xCount, yCount, zCount, xSize / 2.0f, ySize / 2.0f, zSize / 2.0f, gridSize_);
		}
	}

	WorldInternal::~WorldInternal()
	{
		for (size_t i = 0; layers.size(); i++)
		{
			delete layers[i];
		}

		layers.clear();

		for (std::set<Object*>::iterator it = containedObjects.begin(); it != containedObjects.end(); it++)
		{
			(*it)->Release();
		}
	}

	void WorldInternal::AddObject(Object* o)
	{
		SafeAddRef(o);
		containedObjects.insert(o);
		AddObjectInternal(o);
	}

	void WorldInternal::RemoveObject(Object* o)
	{
		RemoveObjectInternal(o);
		containedObjects.erase(o);
		SafeRelease(o);
	}

	void WorldInternal::AddObjectInternal(Object* o)
	{
		assert(o != NULL);

		ObjectInternal* o_ = (ObjectInternal*) o;

		int32_t gridInd = (int32_t)((o_->GetNextStatus().Radius * 2.0f) / gridSize);

		if (gridInd * gridSize < (o_->GetNextStatus().Radius * 2.0f)) gridInd++;

		int32_t ind = 0;
		bool found = false;
		for (size_t i = 0; i < layers.size(); i++)
		{
			if (ind <= gridInd && gridInd < ind * 2)
			{
				if (layers[i]->AddObject(o))
				{
					((ObjectInternal*) o)->SetWorld(this);
					found = true;
				}
				else
				{
					break;
				}
			}
		}

		if (!found)
		{
			((ObjectInternal*) o)->SetWorld(this);
			outofLayers.AddObject(o);
		}
	}

	void WorldInternal::RemoveObjectInternal(Object* o)
	{
		assert(o != NULL);

		ObjectInternal* o_ = (ObjectInternal*) o;

		int32_t gridInd = (int32_t) ((o_->GetCurrentStatus().Radius * 2.0f) / gridSize);

		if (gridInd * gridSize < (o_->GetCurrentStatus().Radius * 2.0f)) gridInd++;

		int32_t ind = 0;
		bool found = false;
		for (size_t i = 0; i < layers.size(); i++)
		{
			if (ind <= gridInd && gridInd < ind * 2)
			{
				if (layers[i]->RemoveObject(o))
				{
					((ObjectInternal*) o)->SetWorld(NULL);
					found = true;
				}
				else
				{
					break;
				}
			}
		}

		if (!found)
		{
			((ObjectInternal*) o)->SetWorld(NULL);
			outofLayers.RemoveObject(o);
		}
	}

	void WorldInternal::Culling(const Matrix44& cameraProjMat, bool isOpenGL)
	{
		objs.clear();
		
		float maxx = 1.0f;
		float minx = -1.0f;

		float maxy = 1.0f;
		float miny = -1.0f;

		float maxz = 1.0f;
		float minz = 0.0f;
		if (isOpenGL) minz = -1.0f;

		const int32_t xdiv = 2;
		const int32_t ydiv = 2;
		const int32_t zdiv = 2;

		for (int32_t z = 0; z < zdiv; z++)
		{
			for (int32_t y = 0; y < ydiv; y++)
			{
				for (int32_t x = 0; x < xdiv; x++)
				{
					float xsize = 1.0f / (float) xdiv;
					float ysize = 1.0f / (float) ydiv;
					float zsize = 1.0f / (float) zdiv;

					float maxx_ = (maxx - minx) * (xsize * (x + 1)) + minx;
					float minx_ = (maxx - minx) * (xsize * (x + 0)) + minx;

					float maxy_ = (maxy - miny) * (ysize * (y + 1)) + miny;
					float miny_ = (maxy - miny) * (ysize * (y + 0)) + miny;

					float maxz_ = (maxz - minz) * (zsize * (z + 1)) + minz;
					float minz_ = (maxz - minz) * (zsize * (z + 0)) + minz;

					Vector3DF eyebox[8];

					eyebox[0 + 0] = Vector3DF(minx_, miny_, maxz);
					eyebox[1 + 0] = Vector3DF(maxx_, miny_, maxz);
					eyebox[2 + 0] = Vector3DF(minx_, maxy_, maxz);
					eyebox[3 + 0] = Vector3DF(maxx_, maxy_, maxz);

					eyebox[0 + 4] = Vector3DF(minx_, miny_, minz);
					eyebox[1 + 4] = Vector3DF(maxx_, miny_, minz);
					eyebox[2 + 4] = Vector3DF(minx_, maxy_, minz);
					eyebox[3 + 4] = Vector3DF(maxx_, maxy_, minz);

					Matrix44 cameraProjMatInv = cameraProjMat;
					cameraProjMatInv.SetInverted();

					for (int32_t i = 0; i < 8; i++)
					{
						eyebox[i] = cameraProjMatInv.Transform3D(eyebox[i]);
					}

					Vector3DF max_(FLT_MIN, FLT_MIN, FLT_MIN);
					Vector3DF min_(FLT_MAX, FLT_MAX, FLT_MAX);

					for (int32_t i = 0; i < 8; i++)
					{
						if (eyebox[i].X > max_.X) max_.X = eyebox[i].X;
						if (eyebox[i].Y > max_.Y) max_.Y = eyebox[i].Y;
						if (eyebox[i].Z > max_.Z) max_.Z = eyebox[i].Z;

						if (eyebox[i].X < min_.X) min_.X = eyebox[i].X;
						if (eyebox[i].Y < min_.Y) min_.Y = eyebox[i].Y;
						if (eyebox[i].Z < min_.Z) min_.Z = eyebox[i].Z;
					}

					/* �͈͓��Ɋ܂܂��O���b�h���擾 */
					for (size_t i = 0; i < layers.size(); i++)
					{
						layers[i]->AddGrids(max_, min_, grids);
					}

					grids.push_back(&outofLayers);
				}
			}
		}

		/* �O���b�h����I�u�W�F�N�g�擾 */
		for (size_t i = 0; i < grids.size(); i++)
		{
			for (size_t j = 0; j < grids[i]->GetObjects().size(); j++)
			{
				Object* o = grids[i]->GetObjects()[j];
				objs.push_back(o);
			}
		}

		/* �擾�����O���b�h��j�� */
		for (size_t i = 0; i < grids.size(); i++)
		{
			grids[i]->IsScanned = false;
		}
	}
}