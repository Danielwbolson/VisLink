using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using UnityEngine.UI;
using System;

public class VisSyncSharedTexture : MonoBehaviour
{
    [DllImport("VLUnityConnector")]
    private static extern int getSharedTexture(int api, string name, int deviceIndex);
    [DllImport("VLUnityConnector")]
    private static extern IntPtr GetCreateTextureFunc();
    [DllImport("VLUnityConnector")]
    private static extern bool isTextureReady(int tex);
    [DllImport("VLUnityConnector")]
    private static extern int getTextureWidth(int textureId);
    [DllImport("VLUnityConnector")]
    private static extern int getTextureHeight(int textureId);
    [DllImport("VLUnityConnector")]
    private static extern int getTextureId(int textureId);

    public string textureName = "test.png";

    private bool initialized = false;
    private bool isTextureRequested = false;
    private int tex;
    private Texture2D externalTex = null;

    public Texture2D GetExternalTexture()
    {
        return externalTex;
    }

    // Update is called once per frame
    void Update()
    {
        if (!initialized)
        {
            if (!isTextureRequested)
            {
                VisSyncClient client = gameObject.GetComponentInParent<VisSyncClient>();
                if (client.IsReady())
                {
                    int api = client.GetAPI();
                    tex = getSharedTexture(api, textureName, 0);
                    GL.IssuePluginEvent(GetCreateTextureFunc(), tex);
                    isTextureRequested = true;
                }
            }

            if (isTextureReady(tex))
            {
                System.IntPtr pointer = new System.IntPtr(getTextureId(tex));
                externalTex = Texture2D.CreateExternalTexture(getTextureWidth(tex), getTextureHeight(tex), TextureFormat.ARGB32, false, false, pointer);
                Debug.Log(getTextureWidth(tex));
                Renderer[] renderers = gameObject.GetComponentsInChildren<Renderer>();
                foreach (Renderer renderer in renderers)
                {
                    renderer.material.mainTexture = externalTex;
                }
                initialized = true;
            }
        }

        
    }
}
