﻿// This file is part of Topologic software library.
// Copyright(C) 2019, Cardiff University and University College London
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Grasshopper.Kernel;
using Grasshopper.Kernel.Types;
using Rhino.Geometry;

namespace TopologicGH
{
    public class DictionaryValueAtKey : GH_Component
    {

        public DictionaryValueAtKey()
          : base("Dictionary.ValueAtKey", "Dictionary.ValueAtKey", "Retrieves the value of a Dictionary at a given key", "Topologic", "Dictionary")
        {
        }

        /// <summary>
        /// Registers all the input parameters for this component.
        /// </summary>
        protected override void RegisterInputParams(GH_Component.GH_InputParamManager pManager)
        {
            pManager.AddGenericParameter("Dictionary", "Dictionary", "Dictionary", GH_ParamAccess.item);
            pManager.AddTextParameter("Key", "Key", "Key", GH_ParamAccess.item);
        }

        /// <summary>
        /// Registers all the output parameters for this component.
        /// </summary>
        protected override void RegisterOutputParams(GH_Component.GH_OutputParamManager pManager)
        {
            pManager.AddGenericParameter("Value", "Value", "Value", GH_ParamAccess.item);
        }

        /// <summary>
        /// This is the method that actually does the work.
        /// </summary>
        /// <param name="DA">The DA object is used to retrieve from inputs and store in outputs.</param>
        protected override void SolveInstance(IGH_DataAccess DA)
        {
            // Declare a variable for the input String
            GH_ObjectWrapper dictionaryGoo = null;
            String key = null;

            // Use the DA object to retrieve the data inside the first input parameter.
            // If the retieval fails (for example if there is no data) we need to abort.
            if (!DA.GetData(0, ref dictionaryGoo)) { return; }
            if (!DA.GetData(1, ref key)) { return; }

            Dictionary<String, Object> dictionary = dictionaryGoo.Value as Dictionary<String, Object>;

            if (dictionary == null)
            {
                Dictionary<Object, Object> ooDictionary = dictionaryGoo.Value as Dictionary<Object, Object>;

                dictionary = new Dictionary<String, Object>();
                foreach (var ooKeyValuePair in ooDictionary)
                {
                    String oKey = ooKeyValuePair.Key as String;
                    if (oKey == null)
                    {
                        throw new Exception("The dictionary contains a non-String key.");
                    }

                    dictionary.Add(oKey, ooKeyValuePair.Value);
                }
            }

            // If the retrieved data is Nothing, we need to abort.
            // We're also going to abort on a zero-length String.
            if (key == null) { return; }
            //if (data.Length == 0) { return; }

            // Convert the String to a character array.
            //char[] chars = data.ToCharArray();


            Object value = Topologic.Dictionary.ValueAtKey(dictionary, key);

            DA.SetData(0, value);
        }

        /// <summary>
        /// Provides an Icon for the component.
        /// </summary>
        protected override System.Drawing.Bitmap Icon
        {
            get
            {
                //You can add image files to your project resources and access them like this:
                // return Resources.IconForThisComponent;
                return Resources.NMT_borderless_logo_small;
            }
        }

        /// <summary>
        /// Gets the unique ID for this component. Do not change this ID after release.
        /// </summary>
        public override Guid ComponentGuid
        {
            get { return new Guid("5522b653-10e4-482a-aea4-17a069ef6920"); }
        }
    }
}
