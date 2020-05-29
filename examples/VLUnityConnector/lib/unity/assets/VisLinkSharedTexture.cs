using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using UnityEngine.UI;
using System;
using UnityEngine.Rendering;
using System.Threading;

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
    private static extern int getSemaphore(int id, string name, int deviceIndex);
    [DllImport("VLUnityConnector")]
    private static extern IntPtr GetCreateSemaphoreFunc();
    [DllImport("VLUnityConnector")]
    private static extern bool isSemaphoreReady(int sem);
    [DllImport("VLUnityConnector")]
    private static extern void semaphoreWriteTexture(int sem, int tex);
    [DllImport("VLUnityConnector")]
    private static extern void semaphoreReadTexture(int sem, int tex);
    [DllImport("VLUnityConnector")]
    private static extern IntPtr GetSemaphoreSignalFunc();
    [DllImport("VLUnityConnector")]
    private static extern IntPtr GetSemaphoreWaitForSignalFunc();
    [DllImport("VLUnityConnector")]
    private static extern int getSemaphoreId(int semId);

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

    [DllImport("VLUnityConnector")]
    private static extern int getThreadId([In, Out] IntPtr data);

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

    public string getThreadText()
    {
        byte[] data = new byte[50];
        GCHandle h = GCHandle.Alloc(data, GCHandleType.Pinned);
        int len = getThreadId(h.AddrOfPinnedObject());
        h.Free();
        return System.Text.Encoding.UTF8.GetString(data).Substring(0, len);
    }

    public string textureName = "test.png";

    private Camera cam = null;
    private bool initialized = false;
    private bool running = false;
    private int tex;
    private int textureReady;
    private int textureComplete;
    private bool isTextureRequested = false;
    private IntPtr startFrame;
    private IntPtr finishFrame;
    int frame = 0;
    Texture2D externalTex = null;

    private int currentFrame = 0;

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
                    startFrame = getMessageQueue(api, textureName+"-start");
                    finishFrame = getMessageQueue(api, textureName + "-finish");
                    tex = getSharedTexture(api, textureName, 0);
                    GL.IssuePluginEvent(GetCreateTextureFunc(), tex);
                    textureReady = getSemaphore(api, textureName + "-ready", 0);
                    textureComplete = getSemaphore(api, textureName + "-complete", 0);
                    isTextureRequested = true;

                    /**/

                }
            }
            else if (isTextureReady(tex) && isSemaphoreReady(textureReady) && isSemaphoreReady(textureComplete))
            {
                //GL.IssuePluginEvent(GetSemaphoreWaitForSignalFunc(), textureReady);
                initialized = true;
            }
            else if (isTextureReady(tex))
            {
                semaphoreWriteTexture(textureReady, tex);
                semaphoreReadTexture(textureComplete, tex);

                GL.IssuePluginEvent(GetCreateSemaphoreFunc(), textureReady);
                GL.IssuePluginEvent(GetCreateSemaphoreFunc(), textureComplete);

                GameObject view = new GameObject("View");
                view.transform.parent = transform;
                cam = view.AddComponent(typeof(Camera)) as Camera;
                RenderTexture rt = new RenderTexture(getTextureWidth(tex), getTextureHeight(tex), 24, RenderTextureFormat.ARGB32);
                //Debug.Log("" + getTextureWidth(tex) + " " + getTextureHeight(tex) + " " +getTextureId(tex));
                rt.Create();
                cam.targetTexture = rt;
                cam.clearFlags = CameraClearFlags.SolidColor; 
                cam.backgroundColor = new Color(0, 0, 0, 0);

                System.IntPtr pointer = new System.IntPtr(getTextureId(tex));
                //Debug.Log(getTextureId(tex));
                externalTex = Texture2D.CreateExternalTexture(getTextureWidth(tex), getTextureHeight(tex), TextureFormat.ARGB32, false, false, pointer);

                //commandBuffer = new CommandBuffer();
                //cam.AddCommandBuffer(CameraEvent.AfterEverything, CommandBuffer);
                CommandBuffer commandBuffer = new CommandBuffer();
                //commandBuffer.IssuePluginEvent(GetCreateSemaphoreFunc(), textureReady);
                //commandBuffer.IssuePluginEvent(GetCreateSemaphoreFunc(), textureComplete);
                commandBuffer.IssuePluginEvent(GetSemaphoreWaitForSignalFunc(), textureReady);
                cam.AddCommandBuffer(CameraEvent.BeforeGBuffer, commandBuffer);
                commandBuffer = new CommandBuffer();
                commandBuffer.CopyTexture(cam.targetTexture, externalTex);
                cam.AddCommandBuffer(CameraEvent.AfterImageEffects, commandBuffer);
                //commandBuffer = new CommandBuffer();
                //commandBuffer.IssuePluginEvent(GetSemaphoreSignalFunc(), textureComplete);
                //cam.AddCommandBuffer(CameraEvent.AfterEverything, commandBuffer);


                //initialized = true;
            }
        }
        else
        {

            waitForMessage(startFrame);
            //semaphoreWaitForSignal(textureReady);
            //Debug.Log(getSemaphoreId(textureReady));
            //GL.IssuePluginEvent(GetSemaphoreWaitForSignalFunc(), textureReady);

            int frameVal = queueRecieveInt(startFrame);
            //Debug.Log(frameVal);
            frameVal = queueRecieveInt(startFrame);
            //Debug.Log(frameVal);
            Matrix4x4 proj = getMatrix4x4(startFrame);
            Matrix4x4 view = getMatrix4x4(startFrame);
            Debug.Log(view);
            Matrix4x4 model = getMatrix4x4(startFrame);

            if (frame == 1)
            {
                //Debug.Log(frame);

                GL.IssuePluginEvent(GetCreateSemaphoreFunc(), textureReady);
                GL.IssuePluginEvent(GetCreateSemaphoreFunc(), textureComplete);
            }
            if (frame >= 1)
            {
                //GL.IssuePluginEvent(GetSemaphoreWaitForSignalFunc(), textureReady);
            }


            Matrix4x4 rh_to_lh = Matrix4x4.identity;
            rh_to_lh[0, 0] = -1.0f;

            cam.projectionMatrix = proj;
            cam.worldToCameraMatrix = view;

            //rh_to_lh[1, 1] = -1;
            cam.worldToCameraMatrix = rh_to_lh * cam.worldToCameraMatrix * model;
            //cam.worldToCameraMatrix = rh_to_lh * cam.worldToCameraMatrix * rh_to_lh * model;



            running = true;

            //sendMessage(finishFrame);

            //Debug.Log(getThreadText());
            Debug.Log(currentFrame);
            currentFrame++;

            //sendMessage(finishFrame);
            //Thread.Sleep(100);
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
                //sendMessage(finishFrame);
                //Debug.Log("test");
                //Debug.Log(getSemaphoreId(textureComplete));
                GL.IssuePluginEvent(GetSemaphoreSignalFunc(), textureComplete);
                //GL.IssuePluginEvent(GetSemaphoreWaitForSignalFunc(), textureReady);
                //semaphoreSignal(textureComplete);
                sendMessage(finishFrame);
                currentFrame = 0;
                frame++;
            }
        }
    }
}
