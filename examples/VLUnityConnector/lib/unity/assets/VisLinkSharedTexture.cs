using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using UnityEngine.UI;
using System;
using UnityEngine.Rendering;

public class VisLinkSharedTexture : MonoBehaviour
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
    [DllImport("VLUnityConnector")]
	private static extern IntPtr getMessageQueue(int api, string name);
    [DllImport("VLUnityConnector")]
	private static extern void waitForMessage(IntPtr msgQueue);
    [DllImport("VLUnityConnector")]
	private static extern void sendMessage(IntPtr msgQueue);
    [DllImport("VLUnityConnector")]
    private static extern int queueRecieveInt(IntPtr msgQueue);
    [DllImport("VLUnityConnector")]
    private static extern void queueRecieveFloatArray(IntPtr msgQueue, [In, Out] IntPtr arr, int size);

    public float[] getFloatArray(IntPtr msgQueue, int size)
    {
        float[] arr = new float[size];
        GCHandle h = GCHandle.Alloc(arr, GCHandleType.Pinned);
        queueRecieveFloatArray(msgQueue, h.AddrOfPinnedObject(), size);
        h.Free();
        return arr;
    }

    public Matrix4x4 getMatrix4x4(IntPtr msgQueue)
    {
        float[] arr = getFloatArray(msgQueue, 16);
        Matrix4x4 m = new Matrix4x4();
        for (int f = 0; f < arr.Length; f++)
        {
            m[f] = arr[f];
        }
        return m;
        //return Matrix4x4.identity;
    }

    public string textureName = "test.png";

    private Camera cam = null;
    private bool initialized = false;
    private bool running = false;
    private int tex;
    private bool isTextureRequested = false;
    private IntPtr startFrame;
    private IntPtr finishFrame;

    // Start is called before the first frame update
    IEnumerator Start()
    {
        yield return StartCoroutine("SyncFrames");
    }

    // Update is called once per frame
    void Update() {
        if (!initialized)
        {
            if (!isTextureRequested)
            {
                VisLinkClient client = transform.parent.GetComponent<VisLinkClient>();
                if (client.IsReady())
                {
                    int api = client.GetAPI();
                    startFrame = getMessageQueue(api, "start");
                    finishFrame = getMessageQueue(api, "finish");
                    tex = getSharedTexture(api, textureName, 0);
                    GL.IssuePluginEvent(GetCreateTextureFunc(), tex);
                    isTextureRequested = true;

                    /**/

                }
            }
            else if (isTextureReady(tex))
            {
                GameObject view = new GameObject("View");
                view.transform.parent = transform;
                cam = view.AddComponent(typeof(Camera)) as Camera;
                RenderTexture rt = new RenderTexture(getTextureWidth(tex), getTextureHeight(tex), 24, RenderTextureFormat.ARGB32);
                Debug.Log("" + getTextureWidth(tex) + " " + getTextureHeight(tex) + " " +getTextureId(tex));
                rt.Create();
                cam.targetTexture = rt;
                cam.clearFlags = CameraClearFlags.SolidColor;
                cam.backgroundColor = new Color(0, 0, 0, 0);

                System.IntPtr pointer = new System.IntPtr(getTextureId(tex));
                Texture2D externalTex = Texture2D.CreateExternalTexture(getTextureWidth(tex), getTextureHeight(tex), TextureFormat.RGBA32, false, false, pointer);
                CommandBuffer commandBuffer = new CommandBuffer();
                commandBuffer.CopyTexture(rt, externalTex);
                cam.AddCommandBuffer(CameraEvent.AfterEverything, commandBuffer);

                initialized = true;
            }
        }
        else
        {
            waitForMessage(startFrame);
            int frame = queueRecieveInt(startFrame);
            Matrix4x4 proj = getMatrix4x4(startFrame);
            //Debug.Log(proj);
            Matrix4x4 view = getMatrix4x4(startFrame);
            //Debug.Log(view);
            Matrix4x4 model = getMatrix4x4(startFrame);
            //Debug.Log(model);

            cam.projectionMatrix = proj;
            cam.worldToCameraMatrix = view;

            Matrix4x4 rh_to_lh = Matrix4x4.identity;
            rh_to_lh[0, 0] = -1;
            //rh_to_lh[1, 1] = -1;
            cam.worldToCameraMatrix = rh_to_lh * cam.worldToCameraMatrix * model;
            //cam.worldToCameraMatrix = cam.worldToCameraMatrix * model;

            //sendMessage(finishFrame);
            //Debug.Log("frame finished");
            running = true;
        }
    }

    private IEnumerator SyncFrames()
    {
        while (true)
        {
            // Wait until all frame rendering is done
            yield return new WaitForEndOfFrame();

            if (running)
            {
                sendMessage(finishFrame);
            }
        }
    }
}
